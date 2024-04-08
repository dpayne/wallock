#include "surface/Surface.hpp"

#include <spdlog/common.h>
#include <wayland-client-protocol.h>
#include <memory>
#include <utility>

#include "display/Display.hpp"
#include "mpv/MpvResource.hpp"
#include "overlay/CairoBarSurface.hpp"
#include "overlay/CairoIndicatorSurface.hpp"
#include "registry/Registry.hpp"
#include "render/Renderer.hpp"
#include "util/Log.hpp"

namespace wall {
class BufferPool;
class Config;
class Keyboard;
enum class State;
}  // namespace wall
struct wl_output;
struct wl_subsurface;
struct wl_surface;

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
}

auto wall::Surface::update_settings() -> void {
    if (m_indicator != nullptr) {
        m_indicator->update_settings();
    }

    if (m_bar != nullptr) {
        m_bar->update_settings();
    }
}

auto wall::Surface::get_mpv_resource() const -> MpvResource* { return m_mpv_resource.get(); }

auto wall::Surface::share_mpv_resource() -> std::shared_ptr<MpvResource> { return m_mpv_resource; }

auto wall::Surface::set_mpv_resource(std::shared_ptr<MpvResource> resource) -> void {
    if (resource != nullptr) {
        LOG_DEBUG("Setting mpv resource for surface {} and mode {}", get_output_name(), static_cast<uint32_t>(get_resource_mode()));
        m_mpv_resource = std::move(resource);
    }
}

auto wall::Surface::copy_mpv_resource() -> std::shared_ptr<MpvResource> { return m_mpv_resource; }

auto wall::Surface::get_registry() const -> Registry* { return m_registry; }

auto wall::Surface::get_display() const -> Display* { return m_display; }

auto wall::Surface::get_output_name() const -> const std::string& { return m_output_name; }

auto wall::Surface::is_primary() const -> bool { return m_is_primary; }

auto wall::Surface::set_is_primary(bool is_primary) -> void { m_is_primary = is_primary; }

auto wall::Surface::is_failed() const -> bool { return m_is_failed; }

auto wall::Surface::set_is_failed(bool failed) -> void { m_is_failed = failed; }

auto wall::Surface::next() -> void {
    if (m_renderer != nullptr) {
        m_renderer->next();
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
        m_bar->draw(get_width(), get_height(), get_scale_factor());
    }

    if (m_indicator != nullptr) {
        m_indicator->on_state_change(state);
        m_indicator->draw(get_width(), get_height(), get_scale_factor());
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
    m_indicator_surface = wl_compositor_create_surface(compositor);
    m_indicator_subsurface = wl_subcompositor_get_subsurface(subcompositor, m_indicator_surface, get_wl_surface());

    wl_subsurface_set_sync(m_indicator_subsurface);

    m_bar_surface = wl_compositor_create_surface(compositor);
    m_bar_subsurface = wl_subcompositor_get_subsurface(subcompositor, m_bar_surface, get_wl_surface());

    wl_subsurface_set_sync(m_bar_subsurface);

    m_indicator = std::make_unique<CairoIndicatorSurface>(get_config(), this, get_subpixel());
    m_bar = std::make_unique<CairoBarSurface>(get_config(), this, get_subpixel());
}

auto wall::Surface::render() -> void {
    if (m_renderer != nullptr) {
        m_renderer->render(this);
    } else {
        LOG_ERROR("No renderer set for surface {} {}", get_output_name(), static_cast<uint32_t>(get_resource_mode()));
    }
}

auto wall::Surface::draw_overlay() -> void {
    if (m_indicator == nullptr || m_bar == nullptr) {
        return;
    }

    get_indicator()->draw(get_width(), get_height(), get_scale_factor());
    get_bar()->draw(get_width(), get_height(), get_scale_factor());
}
