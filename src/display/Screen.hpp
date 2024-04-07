#pragma once

#include <wayland-client-protocol.h>
#include <memory>
#include "State.hpp"
#include "conf/Config.hpp"
#include "mpv/MpvResource.hpp"
#include "registry/Lock.hpp"
#include "wlr-layer-shell-unstable-v1-protocol.h"

namespace wall {

class LockSurface;
class WallpaperSurface;
class RendererCreator;
class Surface;
class Display;
class Registry;

struct OutputGeometryStruct {
    int32_t m_x_pos{};
    int32_t m_y_pos{};
    int32_t m_physical_width{};
    int32_t m_physical_height{};
    enum wl_output_subpixel m_subpixel {};
    std::string m_make{};
    std::string m_model{};
    int32_t m_transform{};
};
using OutputGeometry = struct OutputGeometryStruct;

struct OutputModeStruct {
    uint32_t m_flags{};
    int32_t m_width{};
    int32_t m_height{};
    int32_t m_refresh{};
};

using OutputMode = struct OutputModeStruct;

struct OutputStateStruct {
    uint32_t m_global_name{};
    wl_output* m_output{};
    OutputGeometry m_geometry{};
    OutputMode m_mode{};
    int32_t m_scale{1};
    std::string m_name{};
    std::string m_description{};
};
using OutputState = struct OutputStateStruct;

class Screen {
   public:
    /* get_config(), m_display, this, name, output */
    Screen(const Config& config, Display* display, Registry* registry, uint32_t global_output_name, wl_output* output);
    virtual ~Screen();

    Screen(const Screen&) = delete;
    auto operator=(const Screen&) -> Screen& = delete;
    Screen(Screen&&) = delete;
    auto operator=(Screen&&) -> Screen& = delete;

    auto update_settings() -> void;

    auto next() -> void;

    [[nodiscard]] auto get_output_state() const -> const OutputState&;

    auto create_lock_surface(Lock* lock) -> void;

    auto create_wallpaper_surface() -> void;

    auto get_lock_surface_mut() -> LockSurface*;

    auto get_wallpaper_surface_mut() -> WallpaperSurface*;

    auto destroy_lock_surface() -> void;

    auto destroy_wallpaper_surface() -> void;

    auto swap_lock_to_wallpaper() -> void;

    auto swap_wallpaper_to_lock(Lock* lock) -> void;

    auto on_state_change(State state) -> void;

    [[nodiscard]] auto is_done() const -> bool;

    auto release_output() -> void;

   protected:
    auto create_lock_surface(Lock* lock, std::shared_ptr<MpvResource> mpv_resource) -> void;

    auto create_wallpaper_surface(std::shared_ptr<MpvResource> mpv_resource) -> void;

    auto on_geometry(int32_t x_pos,
                     int32_t y_pos,
                     int32_t physical_width,
                     int32_t physical_height,
                     int32_t subpixel,
                     const char* make,
                     const char* model,
                     int32_t transform) -> void;

    auto on_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh) -> void;

    auto on_done() -> void;

    auto on_scale(int32_t factor) -> void;

    auto on_name(const char* name) -> void;

    auto on_description(const char* description) -> void;

    [[nodiscard]] auto get_config() const -> const Config&;

    auto update_dimensions_for_surfaces() -> void;

    auto update_dimensions_for_surface(Surface* surface) const -> void;

   private:
    static const wl_output_listener k_listener;

    const Config& m_config;

    bool m_is_done{};

    Display* m_display{};

    Registry* m_registry{};

    OutputState m_output_state{};

    std::unique_ptr<LockSurface> m_lock_surface{};

    std::unique_ptr<WallpaperSurface> m_wallpaper_surface{};
};
}  // namespace wall
