#pragma once

#include <cairo.h>
#include "conf/Config.hpp"
#include "overlay/CairoImageElement.hpp"
#include "overlay/Color.hpp"
#include "overlay/ColorSet.hpp"

namespace wall {
class CairoIndicatorElement {
   public:
    CairoIndicatorElement(const Config& config);
    ~CairoIndicatorElement() = default;

    auto update_settings() -> void;

    auto draw(cairo_t* cairo, double center_x, double center_y) -> std::chrono::milliseconds;

    [[nodiscard]] auto get_highlight_start() const -> double;

    [[nodiscard]] auto get_radius() const -> double;

    [[nodiscard]] auto get_thickness() const -> double;

    auto on_state_change(State state) -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    auto draw_inner_circle(cairo_t* cairo, double center_x, double center_y) -> void;

    auto draw_outer_ring(cairo_t* cairo, double center_x, double center_y) -> void;

    auto draw_borders(cairo_t* cairo, double center_x, double center_y) -> void;

    auto draw_highlight(cairo_t* cairo, double center_x, double center_y) -> void;

    auto update_highlight_arc_start() -> void;

   private:
    const Config& m_config;

    int32_t m_ring_radius{};
    int32_t m_ring_thickness{};

    ColorSet m_ring_inner_fill_color{};
    ColorSet m_ring_fill_color{};
    ColorSet m_ring_border_color{};

    bool m_is_ring_enabled{};
    bool m_is_ring_inner_enabled{};
    bool m_is_draw_ring_highlight{};

    State m_highlight_state{State::None};
    Color m_ring_highlight_color_keypress{};
    Color m_ring_highlight_color_backspace{};
    Color m_ring_highlight_border_color{};

    double m_ring_highlight_start{};
    double m_ring_highlight_arc{};
    int32_t m_ring_highlight_arc_thickness{};
    double m_ring_highlight_arc_border_thickness{};

    int32_t m_ring_border_width{};
    int32_t m_ring_inner_border_width{};

    CairoImageElement m_image;

    State m_state{State::None};
};
}  // namespace wall
