#include "display/Display.hpp"
#include <poll.h>
#include <wayland-client-core.h>
#include <xf86drm.h>
#include <xkbcommon/xkbcommon.h>
#include <chrono>
#include "conf/ConfigMacros.hpp"
#include "display/PrimaryDisplayState.hpp"
#include "input/Keyboard.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "pam/PasswordManager.hpp"
#include "registry/Registry.hpp"
#include "surface/LockSurface.hpp"
#include "surface/WallpaperSurface.hpp"
#include "util/Log.hpp"

wall::Display::Display(const Config& config,
                       Loop* loop,
                       bool is_start_locked,
                       std::function<void(void)> on_stop)  // NOLINT *-performance-unnecessary-value-param
    : m_config{config},
      m_loop{loop},
      m_wl_display{wl_display_connect(nullptr)},
      m_registry{nullptr},
      m_lock{nullptr},
      m_is_locked(is_start_locked),
      m_on_key_processor{config, std::make_unique<PasswordManager>(loop), [this](State state) { on_state_change(state); }},
      m_on_stop{std::move(on_stop)} {
    update_settings();
    m_is_nvidia = detect_nvidia();

    if (m_wl_display == nullptr) {
        LOG_FATAL("Failed to connect to display");
    }

    m_registry = std::make_unique<Registry>(get_config(), this, loop, wl_display_get_registry(m_wl_display));
    m_renderer_creator = std::make_unique<RendererCreator>(get_config(), this);
    roundtrip();

    if (m_is_locked) {
        create_lock();
        for (const auto& screen : m_registry->get_screens()) {
            screen->create_lock_surface(m_lock.get());
        }
        roundtrip();
    } else {
        start_pause_timer();
    }

    m_display_poll = m_loop->add_poll(wl_display_get_fd(m_wl_display), static_cast<int16_t>(POLLIN),
                                      [this](loop::Poll*, uint16_t) { m_is_dispatch_pending = true; });
    check_for_failure();
}

wall::Display::~Display() {
    if (m_wl_display != nullptr) {
        wl_display_disconnect(m_wl_display);
        m_wl_display = nullptr;
    }
}

auto wall::Display::is_nvidia() const -> bool { return m_is_nvidia; }

auto wall::Display::create_lock() -> void {
    stop_pause_timer();
    m_lock = m_registry->get_lock();
    if (m_lock == nullptr) {
        LOG_FATAL("Failed to get lock");
    }

    if (m_registry->get_input_inhibit_manager() != nullptr) {
        m_input_inhibitor = zwlr_input_inhibit_manager_v1_get_inhibitor(m_registry->get_input_inhibit_manager());
    } else if (m_is_enforce_input_inhibitor) {
        LOG_FATAL("Enforce input inhibitor is enabled but failed to get input inhibit manager.");
    }

    // setup keyboard input handler
    setup_keyboard_callback();
    m_lock_time = std::chrono::system_clock::now();
}

auto wall::Display::detect_nvidia() -> bool {
    std::array<drmDevicePtr, 64> devices;
    auto device_count = 0;
    auto is_nvidia = false;

    device_count = drmGetDevices2(0, devices.data(), 64);
    if (device_count < 0) {
        LOG_ERROR("Failed to get devices");
        return false;
    }

    for (auto device_ix = 0; device_ix < device_count; device_ix++) {
        if (devices[device_ix]->bustype == DRM_BUS_PCI) {
            // Check if the vendor ID matches NVIDIA's (0x10DE)
            if (devices[device_ix]->deviceinfo.pci->vendor_id == 0x10DE) {
                is_nvidia = true;
                LOG_DEBUG("Detected NVIDIA GPU");
                break;
            }
        }
    }

    drmFreeDevices(devices.data(), device_count);
    return is_nvidia;
}

auto wall::Display::check_for_failure() -> void {
    if (m_registry == nullptr) {
        return;
    }

    for (const auto& screen : m_registry->get_screens()) {
        if (screen->get_lock_surface_mut() != nullptr) {
            if (screen->get_lock_surface_mut()->is_failed()) {
                stop();
                return;
            }
        }

        if (screen->get_wallpaper_surface_mut() != nullptr) {
            if (screen->get_wallpaper_surface_mut()->is_failed()) {
                stop();
                return;
            }
        }
    }
}

auto wall::Display::loop() -> void {
    while (true) {
        while (wl_display_prepare_read(m_wl_display) != 0) {
            wl_display_dispatch_pending(m_wl_display);
        }
        wl_display_flush(m_wl_display);
        m_is_dispatch_pending = false;
        if (!m_loop->run()) {
            break;
        }

        if (!m_is_dispatch_pending) {
            wl_display_cancel_read(m_wl_display);
        } else {
            wl_display_read_events(m_wl_display);
        }

        wl_display_dispatch_pending(m_wl_display);

        if (m_is_swap_lock_to_wallpaper) {
            swap_lock_to_wallpaper();
            m_is_swap_lock_to_wallpaper = false;
            roundtrip();
        }

        if (m_is_swap_wallpaper_to_lock) {
            swap_wallpaper_to_lock();
            m_is_swap_wallpaper_to_lock = false;
            roundtrip();
        }

        m_egl_surfaces_to_be_destroyed.clear();

        // With nvidia's egl-wayland implementation, we have to create and destroy the
        // EGL surface outside the dispatch loop since it will deadlock otherwise.

        for (auto& surface : m_surfaces_to_be_destroyed) {
            LOG_DEBUG("Destroying surface resources");
            surface->destroy_resources();
        }

        m_surfaces_to_be_destroyed.clear();
        m_screens_to_be_destroyed.clear();

        if (m_registry != nullptr) {
            for (const auto& screen : m_registry->get_screens()) {
                recreate_egl_surface(screen->get_lock_surface_mut());
                recreate_egl_surface(screen->get_wallpaper_surface_mut());
            }
        }

        if (m_is_shutting_done) {
            m_renderer_creator = nullptr;
            if (m_display_poll != nullptr) {
                m_display_poll->close();
                m_display_poll = nullptr;
            }
        }
    }
}

auto wall::Display::recreate_egl_surface(Surface* surface) -> void {
    if (surface == nullptr || surface->get_renderer_mut() == nullptr) {
        return;
    }

    auto* renderer = surface->get_renderer_mut();

    if (renderer->is_recreate_egl_suface()) {
        LOG_DEBUG("Recreating egl surface for surface {}", surface->get_output_name());
        add_egl_surface_to_be_destroyed(renderer->move_egl_surface());
        renderer->set_surface_egl(m_renderer_creator->create_egl_surface(surface->get_wl_surface(), surface->get_width(), surface->get_height()));
        renderer->set_is_recreate_egl_surface(false);
        surface->set_mpv_resource(std::make_shared<MpvResource>(get_config(), this, surface));
        surface->get_mpv_resource()->set_surface(surface);

        try {
            surface->get_mpv_resource()->setup();
            surface->render();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create mpv resource: {}", e.what());
            surface->set_is_failed(true);
        }
    }
}

auto wall::Display::add_surface_to_be_destroyed(std::unique_ptr<Surface> surface) -> void {
    if (surface != nullptr && m_is_nvidia) {
        m_surfaces_to_be_destroyed.push_back(std::move(surface));
    }
}

auto wall::Display::add_egl_surface_to_be_destroyed(std::unique_ptr<SurfaceEGL> surface) -> void {
    if (surface != nullptr && m_is_nvidia) {
        LOG_DEBUG("Adding egl surface to be destroyed");
        m_egl_surfaces_to_be_destroyed.push_back(std::move(surface));
    }
}

auto wall::Display::is_configured() const -> bool {
    if (m_registry == nullptr) {
        return false;
    }

    if (m_registry->get_seat_mut()->get_wl_seat() == nullptr) {
        return false;
    }

    if (m_registry->get_lock_manager()->get_lock_manager() == nullptr) {
        return false;
    }

    if (m_registry->get_layer_shell() == nullptr) {
        return false;
    }

    if (m_registry->get_subcompositor() == nullptr) {
        return false;
    }

    if (m_registry->get_compositor() == nullptr) {
        return false;
    }

    if (m_registry->get_buffer_pool_mut()->get_shm() == nullptr) {
        return false;
    }

    if (m_registry->get_screens().empty()) {
        return false;
    }

    for (const auto& screen : m_registry->get_screens()) {
        if (!screen->is_done()) {
            return false;
        }

        auto is_at_least_one_surface_configured = false;
        if (screen->get_lock_surface_mut() != nullptr && screen->get_lock_surface_mut()->is_configured()) {
            is_at_least_one_surface_configured |= true;
        }

        if (screen->get_wallpaper_surface_mut() != nullptr && screen->get_wallpaper_surface_mut()->is_configured()) {
            is_at_least_one_surface_configured |= true;
        }

        if (!is_at_least_one_surface_configured) {
            return false;
        }
    }

    return true;
}

auto wall::Display::get_wl_display() const -> wl_display* { return m_wl_display; }

auto wall::Display::update_settings() -> void {
    m_is_wallpaper_enabled = wall_conf_get(get_config(), wallpaper, enabled);
    m_is_enforce_input_inhibitor = wall_conf_get(get_config(), input_inhibitor, enforce);
    m_is_pause_after_unlock = wall_conf_get(get_config(), wallpaper, pause_after_unlock);
    m_is_dismiss_after_pause = wall_conf_get(get_config(), wallpaper, dismiss_after_pause);
    m_pause_after_unlock_delay = std::chrono::seconds(wall_conf_get(get_config(), wallpaper, pause_after_unlock_delay_secs));
    m_grace_period = std::chrono::seconds(wall_conf_get(get_config(), grace, period_secs));
    m_primary_name_from_config = wall_conf_get(get_config(), monitor, primary);
    m_primary_state.m_primary_name = m_primary_name_from_config;

    if (m_registry != nullptr) {
        for (const auto& screen : m_registry->get_screens()) {
            screen->update_settings();
        }
    }
}

auto wall::Display::get_config() const -> const Config& { return m_config; }

auto wall::Display::get_lock_mut() -> Lock* { return m_lock.get(); }

auto wall::Display::get_primary_state_mut() -> PrimaryDisplayState* { return &m_primary_state; }

auto wall::Display::get_renderer_creator_mut() -> RendererCreator* { return m_renderer_creator.get(); }

auto wall::Display::is_locked() const -> bool { return m_is_locked; }

auto wall::Display::remove_primary(const std::string& name) -> void {
    if (!m_primary_name_from_config.empty()) {
        return;
    }

    std::lock_guard guard{m_primary_state.m_guard};
    if (m_primary_state.m_primary_name == name) {
        m_primary_state.m_primary_name.clear();
    }
}

auto wall::Display::update_primary(const std::string& name) -> void {
    if (!m_primary_name_from_config.empty()) {
        return;
    }

    std::lock_guard guard{m_primary_state.m_guard};
    if (m_primary_state.m_primary_name.empty()) {
        m_primary_state.m_primary_name = name;
    }
}

auto wall::Display::on_state_change(State state) -> void {
    for (const auto& screen : m_registry->get_screens()) {
        screen->on_state_change(state);
    }

    if (state == State::Valid) {
        unlock();
        return;
    }

    if (m_grace_period.count() > 0 && state != State::NoOp && state != State::None) {
        const auto now = std::chrono::system_clock::now();
        if ((now - m_lock_time) < m_grace_period) {
            unlock();
            return;
        }
    }
}

auto wall::Display::next() -> void {
    for (const auto& screen : m_registry->get_screens()) {
        screen->next();
    }
}

auto wall::Display::stop_pause_timer() -> void {
    if (m_pause_timer != nullptr) {
        m_pause_timer->close();
        m_pause_timer = nullptr;
    }
}

auto wall::Display::reload() -> void {
    LOG_DEBUG("Reloading settings");
    update_settings();
}

auto wall::Display::get_loop() const -> Loop* { return m_loop; }

auto wall::Display::stop() -> void {
    stop_pause_timer();
    m_on_key_processor.stop();

    if (m_registry != nullptr) {
        stop_screens();
    }

    m_screens_to_be_destroyed = m_registry->move_screens();

    // note: render creator is destroyed later, after the screens have been destroyed
    m_registry = nullptr;

    m_is_shutting_done = true;

    if (m_on_stop != nullptr) {
        m_on_stop();
        m_on_stop = nullptr;
    }
}

auto wall::Display::stop_screens() -> void {
    for (const auto& screen : m_registry->get_screens()) {
        if (screen->get_lock_surface_mut() != nullptr && screen->get_lock_surface_mut()->get_renderer_mut() != nullptr) {
            screen->get_lock_surface_mut()->get_renderer_mut()->stop();
        }

        if (screen->get_wallpaper_surface_mut() != nullptr && screen->get_wallpaper_surface_mut()->get_renderer_mut() != nullptr) {
            screen->get_wallpaper_surface_mut()->get_renderer_mut()->stop();
        }
    }

    for (const auto& screen : m_registry->get_screens()) {
        if (screen->get_lock_surface_mut() != nullptr) {
            screen->destroy_lock_surface();
        }

        if (screen->get_wallpaper_surface_mut() != nullptr) {
            screen->destroy_wallpaper_surface();
        }
    }
}

auto wall::Display::flush_events() const -> void {
    wl_display_flush(m_wl_display);
    wl_display_dispatch_pending(m_wl_display);
}

auto wall::Display::roundtrip(bool is_exit_on_failure) const -> void {
    const auto retval = wl_display_roundtrip(m_wl_display);
    if (retval == -1) {
        if (is_exit_on_failure) {
            LOG_FATAL("Failed to roundtrip");
        } else {
            LOG_ERROR("Failed to roundtrip");
        }
    }
}

auto wall::Display::setup_keyboard_callback() -> void {  // NOLINT (readability-make-member-function-const)
    if (m_registry == nullptr || m_registry->get_seat_mut() == nullptr || m_registry->get_seat_mut()->get_keyboard_mut() == nullptr) {
        LOG_ERROR("Failed to setup keyboard callback");
        return;
    }

    if (m_keyboard_callback_id != 0) {
        m_registry->get_seat_mut()->get_keyboard_mut()->remove_on_key_callback(m_keyboard_callback_id);
    }

    m_keyboard_callback_id = m_registry->get_seat_mut()->get_keyboard_mut()->add_on_key_callback(
        [](const OnKeyState& key_state) {
            auto* self = static_cast<wall::Display*>(key_state.m_data);
            self->m_on_key_processor.on_key(key_state.m_keysym, key_state.m_utf8_key);
        },
        this);
}

auto wall::Display::swap_wallpaper_to_lock() -> void {
    LOG_DEBUG("Swapping wallpaper to lock");
    const auto is_swap_compatible = MpvResourceConfig::is_resource_modes_compatible(get_config(), ResourceMode::Wallpaper, ResourceMode::Lock);
    for (const auto& screen : m_registry->get_screens()) {
        std::filesystem::path current_file;
        double seek_position = 0.0;

        if (screen->get_wallpaper_surface_mut() != nullptr && screen->get_wallpaper_surface_mut()->get_mpv_resource() != nullptr) {
            auto* resource = screen->get_wallpaper_surface_mut()->get_mpv_resource();

            current_file = resource->get_current_file();
            seek_position = resource->get_seek_position();

            resource->terminate();
        }

        screen->destroy_wallpaper_surface();
        screen->create_lock_surface(m_lock.get());

        if (is_swap_compatible) {
            screen->get_lock_surface_mut()->set_last_file(current_file);
            screen->get_lock_surface_mut()->set_last_seek_position(seek_position);
        }
    }
}

auto wall::Display::lock() -> void {
    m_is_locked = true;
    if (m_lock != nullptr && m_lock->is_locked()) {
        LOG_DEBUG("Already locked");
        return;
    }

    m_is_swap_wallpaper_to_lock = true;
    create_lock();
}

auto wall::Display::swap_lock_to_wallpaper() -> void {
    const auto is_wallpaper_enabled = wall_conf_get(get_config(), wallpaper, enabled);
    if (!is_wallpaper_enabled) {
        return;
    }

    const auto is_swap_compatible = MpvResourceConfig::is_resource_modes_compatible(get_config(), ResourceMode::Wallpaper, ResourceMode::Lock);
    for (const auto& screen : m_registry->get_screens()) {
        std::filesystem::path current_file;
        double seek_position = 0.0;

        if (screen->get_lock_surface_mut() != nullptr && screen->get_lock_surface_mut()->get_mpv_resource() != nullptr) {
            auto* resource = screen->get_lock_surface_mut()->get_mpv_resource();

            current_file = resource->get_current_file();
            seek_position = resource->get_seek_position();

            LOG_ERROR("Current file: {}", current_file.string());
            LOG_ERROR("Seek position: {}", seek_position);

            resource->terminate();
        }

        screen->destroy_lock_surface();
        roundtrip();
        screen->create_wallpaper_surface();

        if (is_swap_compatible) {
            screen->get_wallpaper_surface_mut()->set_last_file(current_file);
            screen->get_wallpaper_surface_mut()->set_last_seek_position(seek_position);
        }
    }
}

auto wall::Display::unlock() -> void {
    m_is_locked = false;

    auto* keyboard = m_registry->get_seat_mut()->get_keyboard_mut();
    keyboard->remove_on_key_callback(m_keyboard_callback_id);
    keyboard->stop_repeat();
    m_keyboard_callback_id = 0;

    m_is_swap_lock_to_wallpaper = true;

    if (m_input_inhibitor != nullptr) {
        zwlr_input_inhibitor_v1_destroy(m_input_inhibitor);
        m_input_inhibitor = nullptr;
    }

    if (m_lock != nullptr) {
        if (m_lock->is_locked()) {
            m_lock->trigger_unlock();
        }
        m_lock = nullptr;
    }

    start_pause_timer();

    const auto is_wallpaper_enabled = wall_conf_get(get_config(), wallpaper, enabled);
    if (is_wallpaper_enabled) {
        if (!m_primary_state.m_lock_files.empty() && m_primary_state.m_lock_files != m_primary_state.m_wallpaper_files) {
            // rotate the lock files
            auto front = m_primary_state.m_lock_files.front();
            m_primary_state.m_lock_files.pop_front();
            m_primary_state.m_lock_files.push_back(std::move(front));
        }
    } else {
        stop();
    }
}

auto wall::Display::start_pause_timer() -> void {
    if (m_is_pause_after_unlock) {
        LOG_DEBUG("Starting pause timer {}", m_pause_after_unlock_delay.count());
        m_pause_timer = m_loop->add_timer(m_pause_after_unlock_delay, std::chrono::milliseconds::zero(), [this](loop::Timer* this_timer) {
            if (m_is_locked) {
                return;
            }

            for (const auto& screen : m_registry->get_screens()) {
                if (screen->get_wallpaper_surface_mut() != nullptr && screen->get_wallpaper_surface_mut()->get_mpv_resource() != nullptr) {
                    screen->get_wallpaper_surface_mut()->get_mpv_resource()->pause();
                }
            }

            if (m_is_dismiss_after_pause) {
                stop();
            }

            this_timer->close();
            m_pause_timer = nullptr;
        });
    }
}
