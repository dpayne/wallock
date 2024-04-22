#include "surface/Surface.hpp"

#include <spdlog/common.h>
#include <wayland-client-protocol.h>
#include <cmath>
#include <memory>
#include <utility>
#include "display/Display.hpp"
#include "fractional-scale-v1-protocol.h"
#include "mpv/MpvResource.hpp"
#include "overlay/CairoBarSurface.hpp"
#include "overlay/CairoIndicatorSurface.hpp"
#include "registry/Registry.hpp"
#include "render/Renderer.hpp"
#include "util/Log.hpp"
#include "viewporter-protocol.h"

namespace wall {
class BufferPool;
class Config;
class Keyboard;
enum class State;
}  // namespace wall
struct wl_output;
struct wl_subsurface;
struct wl_surface;

const wp_fractional_scale_v1_listener wall::Surface::k_fractional_scale_listener = {
    .preferred_scale = [](void* data, struct wp_fractional_scale_v1*, uint32_t scale) {
        auto* self = static_cast<wall::Surface*>(data);
        self->set_fractional_scale(scale);
    }};

wall::Surface::Surface(const Config& config,
                       std::string output_name,  // NOLINT *-performance-unnecessary-value-param
                       Display* display,
                       Registry* registry,
                       wl_output* output)
    : m_config{config}, m_output_name{std::move(output_name)}, m_display{display}, m_registry{registry}, m_output{output} {}

wall::Surface::~Surface() {
    if (m_renderer != nullptr) {
        m_renderer->stop();
    }

    if (m_wp_viewport != nullptr) {
        wp_viewport_destroy(m_wp_viewport);
        m_wp_viewport = nullptr;
    }

    if (m_fractional_scale_obj != nullptr) {
        wp_fractional_scale_v1_destroy(m_fractional_scale_obj);
        m_fractional_scale_obj = nullptr;
    }
}

auto wall::Surface::update_settings() -> void {
    if (m_indicator != nullptr) {
        m_indicator->update_settings();
    }

    if (m_bar != nullptr) {
        m_bar->update_settings();
    }
}

auto wall::Surface::set_fractional_scale(uint32_t scale) -> void {
    if (scale == m_fractional_scale) {
        return;
    }

    m_fractional_scale = scale;
    if (m_non_scaled_width > 0 && m_non_scaled_height > 0) {
        set_width(get_scaled_size(m_non_scaled_width));
        set_height(get_scaled_size(m_non_scaled_height));

        if (m_wp_viewport != nullptr) {
            wp_viewport_set_destination(m_wp_viewport, m_non_scaled_width, m_non_scaled_height);
        }

        if (get_renderer_mut() != nullptr) {
            wl_egl_window_resize(get_renderer_mut()->get_surface_egl_mut()->get_egl_window(), get_width(), get_height(), 0, 0);
        }
    }
}

auto wall::Surface::get_fractional_scale() const -> uint32_t { return m_fractional_scale; }

auto wall::Surface::get_mpv_resource() const -> MpvResource* { return m_mpv_resource.get(); }

auto wall::Surface::share_mpv_resource() -> std::shared_ptr<MpvResource> { return m_mpv_resource; }

auto wall::Surface::set_mpv_resource(std::shared_ptr<MpvResource> resource) -> void {
    if (resource != nullptr) {
        LOG_DEBUG("Setting mpv resource for surface {} and mode {}", get_output_name(), static_cast<uint32_t>(get_resource_mode()));
        m_mpv_resource = std::move(resource);
    }
}

auto wall::Surface::set_next_resource_override(std::filesystem::path resource) -> void { m_next_resource_override = std::move(resource); }

auto wall::Surface::get_next_resource_override() const -> const std::filesystem::path& { return m_next_resource_override; }

auto wall::Surface::copy_mpv_resource() -> std::shared_ptr<MpvResource> { return m_mpv_resource; }

auto wall::Surface::get_registry() const -> Registry* { return m_registry; }

auto wall::Surface::get_display() const -> Display* { return m_display; }

auto wall::Surface::get_output_name() const -> const std::string& { return m_output_name; }

auto wall::Surface::is_primary() const -> bool { return m_is_primary; }

auto wall::Surface::set_is_primary(bool is_primary) -> void { m_is_primary = is_primary; }

auto wall::Surface::is_failed() const -> bool { return m_is_failed; }

auto wall::Surface::set_is_failed(bool failed) -> void { m_is_failed = failed; }

auto wall::Surface::next() -> void {
    if (m_mpv_resource != nullptr) {
        m_mpv_resource->next();
    }
}

auto wall::Surface::destroy_resources() -> void {
    m_renderer = nullptr;
    m_mpv_resource = nullptr;

    if (get_wl_indicator_subsurface() != nullptr) {
        wl_subsurface_destroy(get_wl_indicator_subsurface());
        m_indicator_subsurface = nullptr;
    }

    if (get_wl_indicator_surface() != nullptr) {
        wl_surface_destroy(get_wl_indicator_surface());
        m_indicator_surface = nullptr;
    }

    if (get_wl_bar_subsurface() != nullptr) {
        wl_subsurface_destroy(get_wl_bar_subsurface());
        m_bar_subsurface = nullptr;
    }

    if (get_wl_bar_surface() != nullptr) {
        wl_surface_destroy(get_wl_bar_surface());
        m_bar_surface = nullptr;
    }

    if (m_surface != nullptr) {
        wl_surface_destroy(m_surface);
        m_surface = nullptr;
    }
}

auto wall::Surface::on_state_change(State state) -> void {
    if (m_bar != nullptr) {
        m_bar->on_state_change(state);
        m_bar->draw(m_non_scaled_width, m_non_scaled_height);
    }

    if (m_indicator != nullptr) {
        m_indicator->on_state_change(state);
        m_indicator->draw(m_non_scaled_width, m_non_scaled_height);
    }
}

auto wall::Surface::is_ready_to_draw() -> bool {
    return is_configured() && !is_failed() && get_renderer_mut() != nullptr && get_renderer_mut()->has_buffer() &&
           get_renderer_mut()->get_surface_egl_mut() != nullptr;
}

auto wall::Surface::get_subpixel() const -> wl_output_subpixel { return m_subpixel; }

auto wall::Surface::set_subpixel(wl_output_subpixel subpixel) -> void { m_subpixel = subpixel; }

auto wall::Surface::get_config() const -> const Config& { return m_config; }

auto wall::Surface::get_wl_output() const -> wl_output* { return m_output; }

auto wall::Surface::get_width() const -> uint32_t { return m_width; }

auto wall::Surface::get_height() const -> uint32_t { return m_height; }

auto wall::Surface::get_scale_factor() const -> int32_t { return m_scale_factor; }

auto wall::Surface::get_wl_surface() const -> wl_surface* { return m_surface; }

auto wall::Surface::set_width(uint32_t width) -> void { m_width = width; }

auto wall::Surface::set_height(uint32_t height) -> void { m_height = height; }

auto wall::Surface::set_scale_factor(int32_t scale_factor) -> void { m_scale_factor = scale_factor; }

auto wall::Surface::get_indicator() const -> CairoIndicatorSurface* { return m_indicator.get(); }

auto wall::Surface::get_bar() const -> CairoBarSurface* { return m_bar.get(); }

auto wall::Surface::get_wl_indicator_subsurface() const -> wl_subsurface* { return m_indicator_subsurface; }

auto wall::Surface::get_wl_indicator_surface() const -> wl_surface* { return m_indicator_surface; }

auto wall::Surface::get_wl_bar_subsurface() const -> wl_subsurface* { return m_bar_subsurface; }

auto wall::Surface::get_wl_bar_surface() const -> wl_surface* { return m_bar_surface; }
auto wall::Surface::is_valid() const -> bool { return m_surface != nullptr && m_width > 0 && m_height > 0; }

auto wall::Surface::get_renderer() const -> const Renderer& { return *m_renderer; }

auto wall::Surface::get_renderer_mut() -> Renderer* { return m_renderer.get(); }

auto wall::Surface::is_configured() const -> bool { return m_is_configured; }

auto wall::Surface::set_is_configured(bool is_configured) -> void { m_is_configured = is_configured; }

auto wall::Surface::set_renderer(std::shared_ptr<Renderer> renderer) -> void {
    LOG_DEBUG("Setting renderer for surface {}", get_output_name());
    m_renderer = std::move(renderer);
}

auto wall::Surface::create_surface() -> void {
    auto* compositor = m_registry->get_compositor()->get_wl_compositor();
    auto* subcompositor = m_registry->get_subcompositor()->get_wl_subcompositor();
    m_surface = wl_compositor_create_surface(compositor);
    m_fractional_scale_obj = wp_fractional_scale_manager_v1_get_fractional_scale(m_registry->get_fractional_scale_manager(), m_surface);
    m_wp_viewport = wp_viewporter_get_viewport(m_registry->get_viewporter(), m_surface);
    m_indicator_surface = wl_compositor_create_surface(compositor);
    m_indicator_subsurface = wl_subcompositor_get_subsurface(subcompositor, m_indicator_surface, get_wl_surface());

    wp_fractional_scale_v1_add_listener(m_fractional_scale_obj, &k_fractional_scale_listener, this);
    wl_subsurface_set_sync(m_indicator_subsurface);

    m_bar_surface = wl_compositor_create_surface(compositor);
    m_bar_subsurface = wl_subcompositor_get_subsurface(subcompositor, m_bar_surface, get_wl_surface());

    wl_subsurface_set_sync(m_bar_subsurface);

    m_indicator = std::make_unique<CairoIndicatorSurface>(get_config(), this, get_subpixel());
    m_bar = std::make_unique<CairoBarSurface>(get_config(), this, get_subpixel());
}

auto wall::Surface::get_scaled_size(uint32_t size) const -> uint32_t {
    return std::round(static_cast<double>(size) * (static_cast<double>(m_fractional_scale) / 120.0));
}

auto wall::Surface::on_configure([[maybe_unused]] uint32_t serial, uint32_t width, uint32_t height) -> void {
    LOG_DEBUG("Surface on_configure: {} {} fractional scale {}", width, height, m_fractional_scale);
    m_non_scaled_width = width;
    m_non_scaled_height = height;
    set_width(get_scaled_size(width));
    set_height(get_scaled_size(height));
    wp_viewport_set_destination(m_wp_viewport, width, height);
}

auto wall::Surface::draw_overlay() -> void {
    if (m_indicator == nullptr || m_bar == nullptr) {
        return;
    }

    get_indicator()->draw(m_non_scaled_width, m_non_scaled_height);
    get_bar()->draw(m_non_scaled_width, m_non_scaled_height);
}
