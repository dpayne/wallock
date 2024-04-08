#pragma once

#include "surface/Surface.hpp"
#include "wlr-layer-shell-unstable-v1-protocol.h"

namespace wall {

class MpvResource;

class WallpaperSurface : public Surface {
   public:
    WallpaperSurface(const Config& config, std::string output_name, Display* display, Registry* registry, wl_output* output);
    ~WallpaperSurface() override;

    WallpaperSurface(const WallpaperSurface&) = delete;
    auto operator=(const WallpaperSurface&) -> WallpaperSurface& = delete;
    WallpaperSurface(WallpaperSurface&&) = delete;
    auto operator=(WallpaperSurface&&) -> WallpaperSurface& = delete;

    auto create() -> void;

    [[nodiscard]] auto get_resource_mode() const -> ResourceMode override;

    auto draw_overlay() -> void override;

    auto destroy_resources() -> void override;

   protected:
    auto on_configure(uint32_t serial, uint32_t width, uint32_t height) -> void override;

    auto on_closed() -> void;

   private:
    static const zwlr_layer_surface_v1_listener k_listener;
    zwlr_layer_surface_v1* m_layer_surface{};

    bool m_is_bar_enabled{};
};
}  // namespace wall
