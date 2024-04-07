#include "overlay/CairoAnalogClockElement.hpp"

#include <chrono>
#include <cmath>
#include <compare>
#include <ctime>
#include <numbers>

#include "State.hpp"
#include "conf/ConfigMacros.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::CairoAnalogClockElement::CairoAnalogClockElement(const Config& config) : m_config(config) { update_settings(); }

auto wall::CairoAnalogClockElement::get_config() const -> const Config& { return m_config; }

auto wall::CairoAnalogClockElement::update_settings() -> void {
    m_hour_hand_length = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_hand_length);
    m_minute_hand_length = wall_conf_get(get_config(), lock_indicator, analog_clock_minute_hand_length);
    m_second_hand_length = wall_conf_get(get_config(), lock_indicator, analog_clock_second_hand_length);

    m_hour_hand_thickness = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_hand_thickness);
    m_minute_hand_thickness = wall_conf_get(get_config(), lock_indicator, analog_clock_minute_hand_thickness);
    m_second_hand_thickness = wall_conf_get(get_config(), lock_indicator, analog_clock_second_hand_thickness);

    m_center_radius = wall_conf_get(get_config(), lock_indicator, analog_clock_center_radius);

    m_hour_marker_radius = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_marker_radius);
    m_hour_marker_length = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_marker_length);
    m_hour_marker_thickness = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_marker_thickness);

    m_second_marker_radius = wall_conf_get(get_config(), lock_indicator, analog_clock_second_marker_radius);
    m_second_marker_length = wall_conf_get(get_config(), lock_indicator, analog_clock_second_marker_length);
    m_second_marker_thickness = wall_conf_get(get_config(), lock_indicator, analog_clock_second_marker_thickness);

    m_is_hour_hand_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_hand_enabled);
    m_is_minute_hand_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_minute_hand_enabled);
    m_is_second_hand_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_second_hand_enabled);

    m_is_center_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_center_enabled);

    m_is_hour_marker_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_hour_marker_enabled);
    m_is_second_marker_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_second_marker_enabled);

    update_colors();
}

auto wall::CairoAnalogClockElement::update_colors() -> void {
    m_hands.m_input = wall_conf_get(get_config(), lock_indicator, analog_clock_hand_color_input);
    m_hands.m_cleared = wall_conf_get(get_config(), lock_indicator, analog_clock_hand_color_cleared);
    m_hands.m_caps_lock = wall_conf_get(get_config(), lock_indicator, analog_clock_hand_color_caps_lock);
    m_hands.m_verifying = wall_conf_get(get_config(), lock_indicator, analog_clock_hand_color_verifying);
    m_hands.m_wrong = wall_conf_get(get_config(), lock_indicator, analog_clock_hand_color_wrong);

    m_center.m_input = wall_conf_get(get_config(), lock_indicator, analog_clock_center_color_input);
    m_center.m_cleared = wall_conf_get(get_config(), lock_indicator, analog_clock_center_color_cleared);
    m_center.m_caps_lock = wall_conf_get(get_config(), lock_indicator, analog_clock_center_color_caps_lock);
    m_center.m_verifying = wall_conf_get(get_config(), lock_indicator, analog_clock_center_color_verifying);
    m_center.m_wrong = wall_conf_get(get_config(), lock_indicator, analog_clock_center_color_wrong);

    m_markers.m_input = wall_conf_get(get_config(), lock_indicator, analog_clock_marker_color_input);
    m_markers.m_cleared = wall_conf_get(get_config(), lock_indicator, analog_clock_marker_color_cleared);
    m_markers.m_caps_lock = wall_conf_get(get_config(), lock_indicator, analog_clock_marker_color_caps_lock);
    m_markers.m_verifying = wall_conf_get(get_config(), lock_indicator, analog_clock_marker_color_verifying);
    m_markers.m_wrong = wall_conf_get(get_config(), lock_indicator, analog_clock_marker_color_wrong);
}

auto wall::CairoAnalogClockElement::get_min_redraw_time() const -> std::chrono::milliseconds {
    if (m_is_second_hand_enabled) {
        return std::chrono::milliseconds(1000);
    }

    if (m_is_minute_hand_enabled) {
        return std::chrono::milliseconds(60000);
    }

    if (m_is_hour_hand_enabled) {
        return std::chrono::milliseconds(3600000);
    }

    return std::chrono::milliseconds::zero();
}

auto wall::CairoAnalogClockElement::should_redraw(const std::chrono::milliseconds& time_since_last_draw) const -> bool {
    const auto min_redraw_time = get_min_redraw_time();
    if (get_min_redraw_time().count() > 0) {
        return time_since_last_draw >= min_redraw_time;
    }

    return false;
}

auto wall::CairoAnalogClockElement::draw(cairo_t* cairo, double center_x, double center_y, State indicator_state) const -> std::chrono::milliseconds {
    // draw analog clock arms in the inner circle

    const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto* local_time = std::localtime(&time);

    const auto seconds = local_time->tm_sec;
    const auto minutes = local_time->tm_min;
    const auto hours = local_time->tm_hour;

    const auto second_angle = (seconds * 6.0) * (std::numbers::pi / 180.0);
    const auto minute_angle = (minutes * 6.0) * (std::numbers::pi / 180.0);
    const auto hour_angle = ((hours % 12) * 30.0) * (std::numbers::pi / 180.0) + (minute_angle / 12.0);

    if (m_is_hour_hand_enabled) {
        // draw hour hand
        draw_hand(cairo, center_x, center_y, hour_angle, m_hour_hand_thickness, m_hour_hand_length, m_hands.get(indicator_state));
    }

    if (m_is_minute_hand_enabled) {
        // draw minute hand
        draw_hand(cairo, center_x, center_y, minute_angle, m_minute_hand_thickness, m_minute_hand_length, m_hands.get(indicator_state));
    }

    if (m_is_second_hand_enabled) {
        // draw second hand
        draw_hand(cairo, center_x, center_y, second_angle, m_second_hand_thickness, m_second_hand_length, m_hands.get(indicator_state));
    }

    if (m_is_center_enabled) {
        // draw center circle
        cairo_set_line_width(cairo, 0.0);
        cairo_arc(cairo, center_x, center_y, m_center_radius, 0, 2.0 * std::numbers::pi);
        Color::set_cairo_color(cairo, m_center.get(indicator_state));
        cairo_fill_preserve(cairo);
        cairo_stroke(cairo);
    }

    if (m_is_hour_marker_enabled) {
        draw_markers(cairo, center_x, center_y, 12, m_hour_marker_radius, m_hour_marker_thickness, m_hour_marker_length,
                     m_markers.get(indicator_state));
    }

    if (m_is_second_marker_enabled) {
        draw_markers(cairo, center_x, center_y, 60, m_second_marker_radius, m_second_marker_thickness, m_second_marker_length,
                     m_markers.get(indicator_state));
    }

    return get_min_redraw_time();
}

auto wall::CairoAnalogClockElement::draw_hand(cairo_t* cairo,
                                              double center_x,
                                              double center_y,
                                              double angle,
                                              double thickness,
                                              double length,
                                              const Color& color) const -> void {
    cairo_set_line_width(cairo, thickness);
    cairo_move_to(cairo, center_x, center_y);

    const auto hour_x = center_x + (length * std::sin(angle));
    const auto hour_y = center_y - (length * std::cos(angle));

    cairo_line_to(cairo, hour_x, hour_y);
    Color::set_cairo_color(cairo, color);

    cairo_stroke(cairo);
}

auto wall::CairoAnalogClockElement::draw_markers(cairo_t* cairo,
                                                 double center_x,
                                                 double center_y,
                                                 uint32_t count,
                                                 double marker_radius,
                                                 double thickness,
                                                 double length,
                                                 const Color& /* color*/) const -> void {
    const auto degrees = 360.0 / count;
    // draw hour markers
    for (auto marker_ix = 0U; marker_ix < count; marker_ix++) {
        const auto angle = (marker_ix * degrees) * (std::numbers::pi / 180.0);
        const auto marker_x = center_x + (marker_radius * std::sin(angle));
        const auto marker_y = center_y - (marker_radius * std::cos(angle));

        cairo_set_line_width(cairo, thickness);
        cairo_move_to(cairo, marker_x, marker_y);
        const auto marker_end_x = center_x + ((marker_radius - length) * std::sin(angle));
        const auto marker_end_y = center_y - ((marker_radius - length) * std::cos(angle));

        cairo_line_to(cairo, marker_end_x, marker_end_y);
        Color::set_cairo_color(cairo, m_markers.get(State::Input));

        cairo_stroke(cairo);
    }
}
