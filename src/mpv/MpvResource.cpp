#include "mpv/MpvResource.hpp"

#include <EGL/egl.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <wayland-client-core.h>
#include "conf/ConfigMacros.hpp"
#include "display/Display.hpp"
#include "mpv/MpvEventHandler.hpp"
#include "mpv/MpvFileLoader.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "mpv/MpvScreenshot.hpp"
#include "util/Log.hpp"

#pragma GCC diagnostic push
// Ignore the warning about missing field initializers in the struct, some older versions of the protocol are missing .axis_value120
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

wall::MpvResource::MpvResource(const Config& config,
                               Display* display,
                               Surface* surface)  // NOLINT *-performance-unnecessary-value-param
    : m_config{config},
      m_display{display},
      m_surface{surface},
      m_resource_config{MpvResourceConfig::build_config(config, surface->get_resource_mode())},
      m_mpv{mpv_create()},
      m_event_handler{std::make_unique<MpvEventHandler>(display->get_loop(), m_mpv)},
      m_screenshot{std::make_shared<MpvScreenshot>(config)},
      m_file_loader{std::make_unique<MpvFileLoader>(config,
                                                    display->get_loop(),
                                                    &m_resource_config,
                                                    display->get_primary_state_mut(),
                                                    [&](const std::string& file) { send_mpv_cmd("loadfile", file.c_str()); })} {
    if (m_mpv == nullptr) {
        LOG_FATAL("Couldn't create mpv handle");
    }
}

wall::MpvResource::~MpvResource() {
    LOG_DEBUG("MpvResource destructor");
    terminate();
}

auto wall::MpvResource::play() -> void {
    if (m_mpv_update_async == nullptr) {
        setup_update_callback();
    }

    if (m_is_paused) {
        send_mpv_cmd("set", "pause", "no");
    }
    m_is_paused = false;
}

auto wall::MpvResource::pause() -> void {
    if (!m_is_paused) {
        send_mpv_cmd("set", "pause", "yes");
    }
    m_is_paused = true;
}

auto wall::MpvResource::set_surface(Surface* surface) -> void { m_surface = surface; }

auto wall::MpvResource::get_surface() const -> Surface* { return m_surface; }

auto wall::MpvResource::get_display() const -> Display* { return m_display; }

auto wall::MpvResource::send_mpv_cmd(const char* arg1) const -> void {
    std::array<const char*, 4> cmd_args = {arg1, nullptr, nullptr, nullptr};
    send_mpv_cmd_base(cmd_args);
}

auto wall::MpvResource::send_mpv_cmd(const char* arg1, const char* arg2) const -> void {
    std::array<const char*, 4> cmd_args = {arg1, arg2, nullptr, nullptr};
    send_mpv_cmd_base(cmd_args);
}

auto wall::MpvResource::send_mpv_cmd(const char* arg1, const char* arg2, const char* arg3) const -> void {
    std::array<const char*, 4> cmd_args = {arg1, arg2, arg3, nullptr};
    send_mpv_cmd_base(cmd_args);
}

auto wall::MpvResource::send_mpv_cmd_base(std::array<const char*, 4> args) const -> void {
    if (m_mpv == nullptr) {
        return;
    }

    const auto err_code = mpv_command(m_mpv, args.data());
    if (err_code < 0) {
        LOG_ERROR("mpv command failed: {}", mpv_error_string(err_code));
    }
}

auto wall::MpvResource::get_current_file() const -> const std::filesystem::path& { return m_file_loader->get_current_file(); }

auto wall::MpvResource::terminate() -> void {
    m_event_handlers.clear();
    m_event_handler = nullptr;

    if (m_mpv_update_async != nullptr) {
        m_mpv_update_async->close();
        m_mpv_update_async = nullptr;
    }

    if (m_screenshot != nullptr) {
        m_screenshot->stop();
    }

    if (m_mpv_context != nullptr) {
        mpv_render_context_free(m_mpv_context);
        m_mpv_context = nullptr;
    }

    if (m_mpv != nullptr) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

auto wall::MpvResource::load_new_config(const wall::MpvResourceConfig& new_config) -> void {
    if (m_resource_config == new_config) {
        return;
    }

    m_resource_config = new_config;
    load_mpv_options();
}

auto wall::MpvResource::next() -> void { m_file_loader->load_next_file(); }

auto wall::MpvResource::setup() -> void {
    // force VO
    const auto is_force_software_rendering = wall_conf_get(get_config(), general, force_software_rendering);
    if (is_force_software_rendering) {
        mpv_set_option_string(m_mpv, "vo", "libmpv");
    }

    if (mpv_initialize(m_mpv) < 0) {
        LOG_FATAL("mpv init failed");
    }

#ifdef NDEBUG
    // disable terminal output in release mode
    send_mpv_cmd("set", "terminal", "no");
#else
    if (get_config().is_debug()) {
        send_mpv_cmd("set", "terminal", "yes");
        // setup mpv debug log
        send_mpv_cmd("set", "msg-level", "all=v");
    }
#endif

    mpv_opengl_init_params init_params = {
        .get_proc_address = get_proc_address,
        .get_proc_address_ctx = this,
    };

    if (m_display != nullptr && m_display->get_wl_display() != nullptr) {
        std::array<mpv_render_param, 4> params = {mpv_render_param{MPV_RENDER_PARAM_WL_DISPLAY, m_display->get_wl_display()},
                                                  mpv_render_param{MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL},
                                                  mpv_render_param{MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &init_params},
                                                  mpv_render_param{MPV_RENDER_PARAM_INVALID, nullptr}};

        auto err_code = mpv_render_context_create(&m_mpv_context, m_mpv, params.data());
        if (err_code < 0 || m_mpv_context == nullptr) {
            LOG_FATAL("Couldn't create mpv render context: {}", err_code);
        }
    }

    load_mpv_options();

    setup_event_handlers();

    setup_update_callback();

    next();
}

auto wall::MpvResource::is_single_frame() const -> bool { return m_is_single_frame; }

auto wall::MpvResource::stop() -> void {
    if (m_mpv != nullptr) {
        send_mpv_cmd("stop");
    }

    if (m_mpv_update_async != nullptr) {
        m_mpv_update_async->close();
        m_mpv_update_async = nullptr;
    }
}

auto wall::MpvResource::setup_update_callback() -> void {
    m_mpv_update_async = m_display->get_loop()->add_poll_pipe([this](loop::PollPipe*, const std::vector<uint8_t>& /* buffer */) {
        mpv_render_context_update(get_mpv_context());
        if (m_surface != nullptr && m_surface->get_renderer_mut() != nullptr) {
            m_surface->get_renderer_mut()->set_is_dirty(true);
            m_surface->get_renderer_mut()->render(get_surface());
        }
    });

    mpv_render_context_set_update_callback(
        m_mpv_context,
        [](void* data) {
            auto* resource = (MpvResource*)data;

            if (resource->m_mpv_update_async != nullptr) {
                resource->m_mpv_update_async->write_one();
            }
        },
        this);
}

auto wall::MpvResource::setup_event_handlers() -> void {
    if (m_event_handler != nullptr) {
        m_event_handlers.emplace_back(m_event_handler->add_event_handler(
            MPV_EVENT_FILE_LOADED,
            [](void* data, [[maybe_unused]] uint64_t user_event_id) {
                auto* resource = (MpvResource*)data;
                resource->handle_file_loaded();
            },
            this));
        m_event_handlers.emplace_back(m_event_handler->add_event_handler(
            MPV_EVENT_END_FILE, []([[maybe_unused]] void* data, [[maybe_unused]] uint64_t user_event_id) { LOG_DEBUG("End of file"); }, this));

        m_event_handlers.emplace_back(m_event_handler->add_event_handler(
            MPV_EVENT_SHUTDOWN, []([[maybe_unused]] void* data, [[maybe_unused]] uint64_t user_event_id) { LOG_DEBUG("End of file"); }, this));
    }
}

auto wall::MpvResource::load_mpv_options() -> void {
    // no audio
    if (m_resource_config.m_is_mute) {
        send_mpv_cmd("set", "mute", "yes");
    }

    switch (m_resource_config.m_fit_mode) {
        case FitMode::Fill:
            send_mpv_cmd("set", "panscan", "1.0");
            break;
        case FitMode::Fit:
            send_mpv_cmd("set", "panscan", "0.0");
            break;
        case FitMode::None:
            [[fallthrough]];
        default:
            break;
    }

    // this lets us load the next file while the current one is paused on the last frame
    send_mpv_cmd("set", "idle", "yes");

    // if there is only one file we want to loop it forever
    send_mpv_cmd("set", "loop", "yes");

    // set image display duration to infinite, we use a timer to load next file instead
    send_mpv_cmd("set", "image-display-duration", "inf");

    m_file_loader->set_resource_config(&m_resource_config);
    m_file_loader->load_options();

    m_screenshot->set_resource_config(&m_resource_config);
    m_screenshot->load_options(this);
}

auto wall::MpvResource::get_mpv() const -> mpv_handle* { return m_mpv; }

auto wall::MpvResource::get_mpv_context() const -> mpv_render_context* { return m_mpv_context; }

auto wall::MpvResource::get_config() const -> const Config& { return m_config; }

auto wall::MpvResource::get_resource_config() const -> const MpvResourceConfig& { return m_resource_config; }

auto wall::MpvResource::get_proc_address(void* /* ctx */, const char* name) -> void* {
    return (void*)eglGetProcAddress(name);  // NOLINT
}

auto wall::MpvResource::handle_file_loaded() -> void {
    auto file_duration = 0.0;
    mpv_get_property(m_mpv, "duration", MPV_FORMAT_DOUBLE, &file_duration);
    m_file_loader->setup_load_next_file_timer(file_duration);

    m_is_single_frame = file_duration == 0.0;

    // Only take a screenshot if this is the primary resource
    if (get_surface()->is_primary()) {
        m_screenshot->screenshot(get_current_file(), m_mpv);
    }
}

#pragma GCC diagnostic pop
