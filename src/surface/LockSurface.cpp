#include "surface/LockSurface.hpp"

#include <spdlog/common.h>
#include <wayland-client-protocol.h>
#include <utility>

#include "display/Display.hpp"
#include "ext-session-lock-v1-protocol.h"
#include "overlay/CairoBarSurface.hpp"
#include "overlay/CairoIndicatorSurface.hpp"
#include "registry/Registry.hpp"
#include "render/Renderer.hpp"
#include "render/RendererCreator.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
class Keyboard;
class Subcompositor;
}  // namespace wall
struct ext_session_lock_surface_v1;
struct ext_session_lock_v1;
struct wl_output;

const ext_session_lock_surface_v1_listener wall::LockSurface::k_listener = {
    .configure = [](void* data, struct ext_session_lock_surface_v1* lock_surface, uint32_t serial, uint32_t width, uint32_t height) {
        LOG_DEBUG("Lock surface configure: {} {} {}", serial, width, height);
        if (width == 0U || height == 0U || data == nullptr || width > static_cast<uint32_t>(INT32_MAX) || height > static_cast<uint32_t>(INT32_MAX)) {
            LOG_ERROR("Invalid lock surface configure: {} {} {}", serial, width, height);
            return;
        }

        ext_session_lock_surface_v1_ack_configure(lock_surface, serial);

        auto* surface = static_cast<LockSurface*>(data);
        surface->on_configure(serial, width, height);
    }};

wall::LockSurface::LockSurface(const Config& config, std::string output_name, Display* display, Registry* registry, wl_output* output)
    : Surface(config, std::move(output_name), display, registry, output) {}

wall::LockSurface::~LockSurface() { wall::LockSurface::destroy_resources(); }

auto wall::LockSurface::on_configure(uint32_t /* serial */, uint32_t width, uint32_t height) -> void {
    LOG_DEBUG("Lock surface on_configure: {} {}", width, height);
    set_width(width);
    set_height(height);

    if (is_configured()) {
        // this is a resize request, resize egl surface
        if (get_renderer_mut() != nullptr) {
            wl_egl_window_resize(get_renderer_mut()->get_surface_egl_mut()->get_egl_window(), width * get_scale_factor(), height * get_scale_factor(),
                                 0, 0);
        }
        return;
    }

    if (get_renderer_mut() == nullptr) {
        get_display()->get_renderer_creator_mut()->create_mpv_renderer(this);
    }

    set_is_configured(true);
}

auto wall::LockSurface::destroy_resources() -> void {
    if (m_lock_surface != nullptr) {
        ext_session_lock_surface_v1_destroy(m_lock_surface);
        m_lock_surface = nullptr;
    }

    Surface::destroy_resources();
}

auto wall::LockSurface::get_lock_surface() const -> ext_session_lock_surface_v1* { return m_lock_surface; }

auto wall::LockSurface::draw_overlay() -> void {
    if (m_lock_surface == nullptr) {
        return;
    }

    Surface::draw_overlay();
}

auto wall::LockSurface::get_resource_mode() const -> ResourceMode { return ResourceMode::Lock; }

auto wall::LockSurface::create(ext_session_lock_v1* lock) -> void {
    LOG_DEBUG("Creating lock surface");
    create_surface();
    LOG_DEBUG("Lock surface created");
    m_lock_surface = ext_session_lock_v1_get_lock_surface(lock, get_wl_surface(), get_wl_output());
    auto err_code = ext_session_lock_surface_v1_add_listener(m_lock_surface, &k_listener, this);
    if (err_code != 0) {
        LOG_FATAL("Failed to add listener to lock surface: {}", err_code);
    }

    struct wl_region* region = wl_compositor_create_region(get_registry()->get_compositor()->get_wl_compositor());
    wl_region_add(region, 0, 0, INT32_MAX, INT32_MAX);
    wl_surface_set_opaque_region(get_wl_surface(), region);
    wl_region_destroy(region);
}
