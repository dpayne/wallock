#pragma once

#include <cairo.h>
#include <wayland-client-protocol.h>
#include <chrono>
#include "State.hpp"
#include "conf/Config.hpp"
#include "overlay/CairoFontCache.hpp"
#include "overlay/CairoState.hpp"
#include "registry/BufferPool.hpp"
#include "surface/Surface.hpp"
#include "util/Loop.hpp"

namespace wall {

class CairoSurface {
   public:
    CairoSurface(const Config& config, Surface* surface, wl_output_subpixel subpixel);
    virtual ~CairoSurface();

    CairoSurface(const CairoSurface&) = delete;
    auto operator=(const CairoSurface&) -> CairoSurface& = delete;

    CairoSurface(CairoSurface&&) = delete;
    auto operator=(CairoSurface&&) -> CairoSurface& = delete;

    auto draw(int32_t width, int32_t height, int32_t scale) -> void;

    virtual auto on_state_change(State state) -> void;

    [[nodiscard]] auto get_pixel_width() const -> int32_t;

   protected:
    static auto wl_subpixel_to_cairo_subpixel(wl_output_subpixel subpixel) -> cairo_subpixel_order_t;

    virtual auto draw_frame(int32_t width, int32_t height, int32_t scale) -> std::chrono::milliseconds = 0;

    auto setup_redraw_timer(std::chrono::milliseconds min_redraw_time) -> void;

    [[nodiscard]] auto should_draw_frame_on_idle(wl_surface* surface, wl_subsurface* subsurface) -> bool;

    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto get_surface() const -> Surface*;

    [[nodiscard]] auto get_cairo_state() const -> CairoState*;

    [[nodiscard]] auto get_font_cache() const -> const CairoFontCache&;

    [[nodiscard]] auto get_font_cache_mut() -> CairoFontCache*;

    [[nodiscard]] auto get_now() const -> std::chrono::time_point<std::chrono::system_clock>;

    [[nodiscard]] auto get_state() const -> State;

    [[nodiscard]] auto get_last_draw_time() const -> std::chrono::time_point<std::chrono::system_clock>;

    [[nodiscard]] auto get_last_activity_time() const -> std::chrono::time_point<std::chrono::system_clock>;

    auto set_now(std::chrono::time_point<std::chrono::system_clock> now) -> void;

    auto set_idle_timeout(std::chrono::milliseconds idle_timeout) -> void;

    auto set_is_visible_on_idle(bool is_visible_on_idle) -> void;

    auto set_state(State state) -> void;

    auto set_last_draw_time(std::chrono::time_point<std::chrono::system_clock> now) -> void;

    auto set_last_activity_time(std::chrono::time_point<std::chrono::system_clock> now) -> void;

    virtual auto update_surface(wl_surface* child_surface, wl_subsurface* subsurface, int32_t subsurf_xpos, int32_t subsurf_ypos, Buffer* buffer)
        -> void;

    auto create_cairo_surface(int32_t width, int32_t height, int32_t pixel_size) -> void;

    auto clear() -> void;

    auto clear_surface(wl_surface* surface, wl_subsurface* subsurface) -> void;

   private:
    const Config& m_config;

    CairoFontCache m_font_cache;

    Surface* m_surface{};

    loop::Timer* m_redraw_timer{};

    std::unique_ptr<CairoState> m_cairo_state{};

    int32_t m_scale{1};

    std::chrono::milliseconds m_idle_timeout{};

    bool m_is_visible_on_idle{};

    State m_state{};

    std::chrono::time_point<std::chrono::system_clock> m_now{};
    std::chrono::time_point<std::chrono::system_clock> m_last_activity_time{};
    std::chrono::time_point<std::chrono::system_clock> m_last_draw_time{};
};
}  // namespace wall
