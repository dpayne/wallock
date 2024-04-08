#include "surface/WallpaperSurface.hpp"

#include <spdlog/common.h>
#include <wayland-client-protocol.h>
#include <utility>

#include "conf/ConfigMacros.hpp"
#include "display/Display.hpp"
#include "display/Screen.hpp"
#include "overlay/CairoBarSurface.hpp"
#include "overlay/CairoIndicatorSurface.hpp"
#include "registry/Registry.hpp"
#include "render/Renderer.hpp"
#include "render/RendererCreator.hpp"
#include "util/Log.hpp"
#include "wlr-layer-shell-unstable-v1-protocol.h"

namespace wall {
class BufferPool;
class Config;
class Keyboard;
class Subcompositor;
}  // namespace wall
struct wl_output;
struct zwlr_layer_surface_v1;

const zwlr_layer_surface_v1_listener wall::WallpaperSurface::k_listener = {
    .configure =
        [](void* data, zwlr_layer_surface_v1* /* surface */, uint32_t serial, uint32_t width, uint32_t height) {
            auto* self = static_cast<wall::WallpaperSurface*>(data);
            self->on_configure(serial, width, height);
        },
    .closed =
        [](void* data, zwlr_layer_surface_v1* /* surface */) {
            auto* self = static_cast<wall::WallpaperSurface*>(data);
            self->on_closed();
        },
};

wall::WallpaperSurface::WallpaperSurface(const Config& config, std::string output_name, Display* display, Registry* registry, wl_output* output)
    : Surface(config, std::move(output_name), display, registry, output) {
    m_is_bar_enabled = wall_conf_get(get_config(), wallpaper, bar_enabled);
}

wall::WallpaperSurface::~WallpaperSurface() { wall::WallpaperSurface::destroy_resources(); }

auto wall::WallpaperSurface::draw_overlay() -> void {
    if (m_layer_surface == nullptr) {
        return;
    }

    return Surface::draw_overlay();
}

auto wall::WallpaperSurface::destroy_resources() -> void {
    if (m_layer_surface != nullptr) {
        zwlr_layer_surface_v1_destroy(m_layer_surface);
        m_layer_surface = nullptr;
    }

    Surface::destroy_resources();
}

auto wall::WallpaperSurface::get_resource_mode() const -> ResourceMode { return ResourceMode::Wallpaper; }

auto wall::WallpaperSurface::create() -> void {
    LOG_DEBUG("Creating wallpaper surface");
    // Already created
    if (get_wl_surface() != nullptr) {
        LOG_DEBUG("Wallpaper surface already created");
        return;
    }

    create_surface();

    // Empty input region
    struct wl_region* input_region = wl_compositor_create_region(get_registry()->get_compositor()->get_wl_compositor());
    wl_surface_set_input_region(get_wl_surface(), input_region);
    wl_region_destroy(input_region);

    m_layer_surface = zwlr_layer_shell_v1_get_layer_surface(get_registry()->get_layer_shell()->get_wlr_layer_shell(), get_wl_surface(),
                                                            get_wl_output(), ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wallock");

    zwlr_layer_surface_v1_set_size(m_layer_surface, 0, 0);
    zwlr_layer_surface_v1_set_anchor(m_layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                                                          ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(m_layer_surface, -1);
    zwlr_layer_surface_v1_add_listener(m_layer_surface, &k_listener, this);
    wl_surface_commit(get_wl_surface());
}

auto wall::WallpaperSurface::on_configure(uint32_t serial, uint32_t width, uint32_t height) -> void {
    LOG_DEBUG("Wallpaper surface configure {} {}", width, height);
    set_width(width);
    set_height(height);

    if (is_configured()) {
        LOG_DEBUG("Resize request for wallpaper surface {} {}", width, height);
        // this is a resize request, resize egl surface
        if (get_renderer_mut() != nullptr) {
            wl_egl_window_resize(get_renderer_mut()->get_surface_egl_mut()->get_egl_window(), width * get_scale_factor(), height * get_scale_factor(),
                                 0, 0);
        }
        return;
    }

    zwlr_layer_surface_v1_ack_configure(m_layer_surface, serial);

    if (get_renderer_mut() == nullptr) {
        get_display()->get_renderer_creator_mut()->create_mpv_renderer(this);
    }

    set_is_configured(true);
}

auto wall::WallpaperSurface::on_closed() -> void { LOG_DEBUG("Wallpaper surface closed"); }
