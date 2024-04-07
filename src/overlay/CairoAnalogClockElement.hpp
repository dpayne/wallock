#pragma once

#include "conf/Config.hpp"
#include "overlay/ColorSet.hpp"

#include <cairo.h>
#include <chrono>

namespace wall {
class CairoAnalogClockElement {
   public:
    CairoAnalogClockElement(const Config& config);

    auto draw(cairo_t* cairo, double center_x, double center_y, State indicator_state) const -> std::chrono::milliseconds;

    [[nodiscard]] auto should_redraw(const std::chrono::milliseconds& time_since_last_draw) const -> bool;

    auto update_settings() -> void;

    auto update_colors() -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto get_min_redraw_time() const -> std::chrono::milliseconds;

    auto draw_hand(cairo_t* cairo, double center_x, double center_y, double angle, double thickness, double length, const Color& color) const -> void;

    auto draw_markers(cairo_t* cairo,
                      double center_x,
                      double center_y,
                      uint32_t count,
                      double marker_radius,
                      double thickness,
                      double length,
                      const Color& color) const -> void;

   private:
    const Config& m_config;

    double m_hour_hand_length{50};
    double m_minute_hand_length{55};
    double m_second_hand_length{60};

    double m_hour_hand_thickness{3};
    double m_minute_hand_thickness{2};
    double m_second_hand_thickness{1};

    double m_center_radius{5};

    double m_hour_marker_radius{60};
    double m_hour_marker_length{5};
    double m_hour_marker_thickness{1};

    double m_second_marker_radius{60};
    double m_second_marker_length{1};
    double m_second_marker_thickness{1};

    ColorSet m_hands;
    ColorSet m_center;
    ColorSet m_markers;

    bool m_is_hour_hand_enabled{true};
    bool m_is_minute_hand_enabled{true};
    bool m_is_second_hand_enabled{true};

    bool m_is_center_enabled{true};

    bool m_is_hour_marker_enabled{true};
    bool m_is_second_marker_enabled{true};
};
}  // namespace wall
