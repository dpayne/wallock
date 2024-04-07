#include "display/Screen.hpp"

#include <spdlog/common.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include "display/Display.hpp"
#include "display/PrimaryDisplayState.hpp"
#include "mpv/MpvResource.hpp"
#include "registry/Lock.hpp"
#include "render/Renderer.hpp"
#include "surface/LockSurface.hpp"
#include "surface/Surface.hpp"
#include "surface/WallpaperSurface.hpp"
#include "util/Log.hpp"

namespace wall {
class BufferPool;
class Compositor;
class Config;
class LayerShell;
class Subcompositor;
enum class State;
}  // namespace wall
struct wl_output;

const wl_output_listener wall::Screen::k_listener = {
    .geometry =
        [](void* data,
           wl_output* /* output */,
           int32_t x_pos,
           int32_t y_pos,
           int32_t physical_width,
           int32_t physical_height,
           int32_t subpixel,
           const char* make,
           const char* model,
           int32_t transform) {
            auto& self = *static_cast<wall::Screen*>(data);
            self.on_geometry(x_pos, y_pos, physical_width, physical_height, subpixel, make, model, transform);
        },
    .mode =
        [](void* data, wl_output* /* output */, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
            auto& self = *static_cast<wall::Screen*>(data);
            self.on_mode(flags, width, height, refresh);
        },
    .done =
        [](void* data, wl_output* /* output */) {
            auto& self = *static_cast<wall::Screen*>(data);
            self.on_done();
        },
    .scale =
        [](void* data, wl_output* /* output */, int32_t factor) {
            auto& self = *static_cast<wall::Screen*>(data);
            self.on_scale(factor);
        },
    .name =
        [](void* data, wl_output* /* output */, const char* name) {
            auto& self = *static_cast<wall::Screen*>(data);
            self.on_name(name);
        },
    .description =
        [](void* data, wl_output* /* output */, const char* description) {
            auto& self = *static_cast<wall::Screen*>(data);
            self.on_description(description);
        },
};

wall::Screen::Screen(const Config& config, Display* display, Registry* registry, uint32_t global_output_name, wl_output* output)
    : m_config{config}, m_display{display}, m_registry{registry}, m_output_state{global_output_name, output} {
    wl_output_add_listener(output, &k_listener, this);
}

wall::Screen::~Screen() {
    m_display->remove_primary(m_output_state.m_name);
    if (m_output_state.m_output != nullptr) {
        wl_output_release(m_output_state.m_output);
        m_output_state.m_output = nullptr;
    }

    if (m_lock_surface != nullptr) {
        m_display->add_surface_to_be_destroyed(std::move(m_lock_surface));
        m_lock_surface = nullptr;
    }

    if (m_wallpaper_surface != nullptr) {
        m_display->add_surface_to_be_destroyed(std::move(m_wallpaper_surface));
        m_wallpaper_surface = nullptr;
    }
}

auto wall::Screen::release_output() -> void {
    if (m_output_state.m_output != nullptr) {
        wl_output_release(m_output_state.m_output);
        m_output_state.m_output = nullptr;
    }
}

auto wall::Screen::update_settings() -> void {
    m_display->update_primary(m_output_state.m_name);

    if (m_lock_surface != nullptr) {
        m_lock_surface->update_settings();
    }

    if (m_wallpaper_surface != nullptr) {
        m_wallpaper_surface->update_settings();
    }
}

auto wall::Screen::next() -> void {
    if (m_lock_surface != nullptr) {
        m_lock_surface->next();
    }

    if (m_wallpaper_surface != nullptr) {
        m_wallpaper_surface->next();
    }
}

auto wall::Screen::get_config() const -> const Config& { return m_config; }

auto wall::Screen::get_output_state() const -> const OutputState& { return m_output_state; }

auto wall::Screen::get_lock_surface_mut() -> LockSurface* { return m_lock_surface.get(); }

auto wall::Screen::get_wallpaper_surface_mut() -> WallpaperSurface* { return m_wallpaper_surface.get(); }

auto wall::Screen::on_geometry(int32_t x_pos,
                               int32_t y_pos,
                               int32_t physical_width,
                               int32_t physical_height,
                               int32_t subpixel,
                               const char* make,
                               const char* model,
                               int32_t transform) -> void {
    m_output_state.m_geometry = {x_pos, y_pos, physical_width, physical_height, static_cast<enum wl_output_subpixel>(subpixel),
                                 make,  model, transform};
    update_dimensions_for_surfaces();
}

auto wall::Screen::on_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh) -> void {
    LOG_DEBUG("Screen::on_mode: flags: {}, width: {}, height: {}, refresh: {}", flags, width, height, refresh);
    m_output_state.m_mode = {flags, width, height, refresh};

    update_dimensions_for_surfaces();
}

auto wall::Screen::is_done() const -> bool { return m_is_done; }

auto wall::Screen::on_done() -> void {
    if (m_is_done) {
        LOG_DEBUG("Screen::on_done: Already done");
        return;
    }

    LOG_DEBUG("Screen::on_done");
    if (m_display->is_locked()) {
        if (m_display->get_lock_mut() != nullptr) {
            create_lock_surface(m_display->get_lock_mut());
        } else {
            LOG_ERROR("Screen::on_done: Lock is nullptr");
        }
    } else {
        create_wallpaper_surface();
    }
    m_is_done = true;
}

auto wall::Screen::on_scale(int32_t factor) -> void {
    LOG_DEBUG("Screen::on_scale: factor: {}", factor);

    update_dimensions_for_surfaces();
}

auto wall::Screen::on_name(const char* name) -> void {
    if (name != nullptr) {
        LOG_DEBUG("Screen::on_name: name: {}", name);
        m_output_state.m_name = name;
        m_display->update_primary(m_output_state.m_name);
    }
}

auto wall::Screen::on_description(const char* description) -> void {
    if (description != nullptr) {
        LOG_DEBUG("Screen::on_description: description: {}", description);
        m_output_state.m_description = description;
    }
}

auto wall::Screen::create_lock_surface(Lock* lock) -> void { create_lock_surface(lock, nullptr); }

auto wall::Screen::create_lock_surface(Lock* lock, std::shared_ptr<MpvResource> mpv_resource) -> void {
    if (m_lock_surface != nullptr) {
        LOG_DEBUG("Lock surface already exists");
        return;
    }

    LOG_DEBUG("Screen::create_lock_surface: {}", m_output_state.m_name);
    m_lock_surface = std::make_unique<LockSurface>(m_config, m_output_state.m_name, m_display, m_registry, m_output_state.m_output);
    update_dimensions_for_surfaces();

    m_display->update_primary(m_output_state.m_name);
    m_lock_surface->set_is_primary(m_output_state.m_name == m_display->get_primary_state_mut()->m_primary_name);

    m_lock_surface->set_mpv_resource(std::move(mpv_resource));
    m_lock_surface->create(lock->get_lock());
}

auto wall::Screen::create_wallpaper_surface() -> void { create_wallpaper_surface(nullptr); }

auto wall::Screen::create_wallpaper_surface(std::shared_ptr<MpvResource> mpv_resource) -> void {
    if (m_wallpaper_surface != nullptr) {
        LOG_DEBUG("Wallpaper surface already exists");
        return;
    }

    LOG_DEBUG("Screen::create_wallpaper_surface: {}", m_output_state.m_name);
    m_wallpaper_surface = std::make_unique<WallpaperSurface>(m_config, m_output_state.m_name, m_display, m_registry, m_output_state.m_output);
    update_dimensions_for_surfaces();
    m_wallpaper_surface->set_mpv_resource(std::move(mpv_resource));

    m_display->update_primary(m_output_state.m_name);
    m_wallpaper_surface->set_is_primary(m_output_state.m_name == m_display->get_primary_state_mut()->m_primary_name);

    m_wallpaper_surface->create();
}

auto wall::Screen::update_dimensions_for_surface(Surface* surface) const -> void {
    if (surface == nullptr) {
        return;
    }

    surface->set_subpixel(m_output_state.m_geometry.m_subpixel);
    surface->set_scale_factor(m_output_state.m_scale);
}
auto wall::Screen::update_dimensions_for_surfaces() -> void {
    update_dimensions_for_surface(m_lock_surface.get());
    update_dimensions_for_surface(m_wallpaper_surface.get());
}

auto wall::Screen::on_state_change(State state) -> void {
    if (m_lock_surface != nullptr) {
        m_lock_surface->on_state_change(state);
    }

    if (m_wallpaper_surface != nullptr) {
        m_wallpaper_surface->on_state_change(state);
    }
}

auto wall::Screen::swap_lock_to_wallpaper() -> void {
    m_lock_surface->get_renderer_mut()->stop();
    create_wallpaper_surface(m_lock_surface->copy_mpv_resource());
}

auto wall::Screen::swap_wallpaper_to_lock(Lock* lock) -> void {
    m_wallpaper_surface->get_renderer_mut()->stop();
    create_lock_surface(lock, m_wallpaper_surface->copy_mpv_resource());
}

auto wall::Screen::destroy_lock_surface() -> void {
    m_display->add_surface_to_be_destroyed(std::move(m_lock_surface));
    m_lock_surface = nullptr;
}

auto wall::Screen::destroy_wallpaper_surface() -> void {
    m_display->add_surface_to_be_destroyed(std::move(m_wallpaper_surface));
    m_wallpaper_surface = nullptr;
}
