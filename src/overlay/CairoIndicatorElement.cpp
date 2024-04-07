#include "overlay/CairoIndicatorElement.hpp"

#include <cstdlib>
#include <numbers>

#include "State.hpp"
#include "conf/ConfigMacros.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::CairoIndicatorElement::CairoIndicatorElement(const Config& config) : m_config{config}, m_image{config} { update_settings(); }

auto wall::CairoIndicatorElement::update_settings() -> void {
    m_is_ring_enabled = wall_conf_get(get_config(), lock_indicator, ring_enabled);
    m_ring_radius = wall_conf_get(get_config(), lock_indicator, ring_radius);
    m_ring_thickness = wall_conf_get(get_config(), lock_indicator, ring_thickness);
    m_ring_border_width = wall_conf_get_with_fallback(get_config(), lock_indicator, ring_border_width, border, width);
    m_ring_inner_border_width = wall_conf_get_with_fallback(get_config(), lock_indicator, ring_inner_border_width, border, width);
    m_ring_highlight_arc = wall_conf_get(get_config(), lock_indicator, ring_highlight_arc);
    m_ring_highlight_arc_border_thickness = wall_conf_get(get_config(), lock_indicator, ring_highlight_arc_border_thickness);
    m_ring_highlight_arc_thickness = wall_conf_get(get_config(), lock_indicator, ring_highlight_arc_thickness);

    m_ring_fill_color.m_input = wall_conf_get(get_config(), lock_indicator, ring_fill_color_input);
    m_ring_fill_color.m_cleared = wall_conf_get(get_config(), lock_indicator, ring_fill_color_cleared);
    m_ring_fill_color.m_caps_lock = wall_conf_get(get_config(), lock_indicator, ring_fill_color_caps_lock);
    m_ring_fill_color.m_verifying = wall_conf_get(get_config(), lock_indicator, ring_fill_color_verifying);
    m_ring_fill_color.m_wrong = wall_conf_get(get_config(), lock_indicator, ring_fill_color_wrong);

    m_ring_border_color.m_input = wall_conf_get_with_fallback(get_config(), lock_indicator, ring_border_color_input, border, color);
    m_ring_border_color.m_cleared = wall_conf_get(get_config(), lock_indicator, ring_border_color_cleared);
    m_ring_border_color.m_caps_lock = wall_conf_get(get_config(), lock_indicator, ring_border_color_caps_lock);
    m_ring_border_color.m_verifying = wall_conf_get(get_config(), lock_indicator, ring_border_color_verifying);
    m_ring_border_color.m_wrong = wall_conf_get(get_config(), lock_indicator, ring_border_color_wrong);

    m_is_ring_inner_enabled = wall_conf_get(get_config(), lock_indicator, ring_inner_enabled);
    m_ring_inner_fill_color.m_input = wall_conf_get_with_fallback(get_config(), lock_indicator, ring_inner_fill_color_input, background, color);
    m_ring_inner_fill_color.m_cleared = wall_conf_get(get_config(), lock_indicator, ring_inner_fill_color_cleared);
    m_ring_inner_fill_color.m_caps_lock = wall_conf_get(get_config(), lock_indicator, ring_inner_fill_color_caps_lock);
    m_ring_inner_fill_color.m_verifying = wall_conf_get(get_config(), lock_indicator, ring_inner_fill_color_verifying);
    m_ring_inner_fill_color.m_wrong = wall_conf_get(get_config(), lock_indicator, ring_inner_fill_color_wrong);

    m_ring_highlight_color_keypress = wall_conf_get(get_config(), lock_indicator, ring_highlight_color_keypress);
    m_ring_highlight_color_backspace = wall_conf_get(get_config(), lock_indicator, ring_highlight_color_backspace);
    m_ring_highlight_border_color = wall_conf_get_with_fallback(get_config(), lock_indicator, ring_highlight_border_color, border, color);

    m_image.update_settings();
}

auto wall::CairoIndicatorElement::get_radius() const -> double { return m_ring_radius; }

auto wall::CairoIndicatorElement::get_thickness() const -> double { return m_ring_thickness; }

auto wall::CairoIndicatorElement::get_config() const -> const Config& { return m_config; }

auto wall::CairoIndicatorElement::get_highlight_start() const -> double { return m_ring_highlight_start; }

auto wall::CairoIndicatorElement::on_state_change(State state) -> void {
    m_state = state;
    switch (state) {
        case State::None:
            [[fallthrough]];
        case State::NoOp:
            break;
        case State::Backspace:
        case State::Keypress:
            m_is_draw_ring_highlight = true;
            update_highlight_arc_start();
            m_highlight_state = state;
            break;
        case State::Wrong:
            [[fallthrough]];
        case State::Cleared:
            [[fallthrough]];
        case State::Verifying:
            [[fallthrough]];
        case State::Idle:
            [[fallthrough]];
        case State::Valid:
            m_highlight_state = State::Idle;
            m_is_draw_ring_highlight = false;
            [[fallthrough]];
        case State::Input:
            [[fallthrough]];
        case State::CapsLock:
            [[fallthrough]];
        default:
            break;
    }
}

auto wall::CairoIndicatorElement::update_highlight_arc_start() -> void { m_ring_highlight_start = ((rand() % 1024) + 512) % 2048; }

auto wall::CairoIndicatorElement::draw(cairo_t* cairo, double center_x, double center_y) -> std::chrono::milliseconds {
    draw_inner_circle(cairo, center_x, center_y);
    m_image.draw(cairo, center_x, center_y, get_radius());
    draw_outer_ring(cairo, center_x, center_y);
    draw_highlight(cairo, center_x, center_y);
    draw_borders(cairo, center_x, center_y);

    return std::chrono::milliseconds::zero();
}

auto wall::CairoIndicatorElement::draw_inner_circle(cairo_t* cairo, double center_x, double center_y) -> void {
    if (!m_is_ring_inner_enabled) {
        return;
    }

    cairo_set_line_width(cairo, 0.0);
    cairo_arc(cairo, center_x, center_y, get_radius() - (get_thickness() / 2.0), 0, 2.0 * std::numbers::pi);
    Color::set_cairo_color(cairo, m_ring_inner_fill_color.get(m_state));
    cairo_fill_preserve(cairo);
    cairo_stroke(cairo);
}

auto wall::CairoIndicatorElement::draw_outer_ring(cairo_t* cairo, double center_x, double center_y) -> void {
    if (!m_is_ring_enabled) {
        return;
    }

    cairo_set_line_width(cairo, get_thickness());
    cairo_arc(cairo, center_x, center_y, get_radius(), 0.0, 2.0 * std::numbers::pi);
    Color::set_cairo_color(cairo, m_ring_fill_color.get(m_state));
    cairo_stroke(cairo);
}

auto wall::CairoIndicatorElement::draw_borders(cairo_t* cairo, double center_x, double center_y) -> void {
    // draw outer border
    Color::set_cairo_color(cairo, m_ring_border_color.get(m_state));
    cairo_set_line_width(cairo, m_ring_border_width);
    cairo_arc(cairo, center_x, center_y, get_radius() + get_thickness() / 2.0, 0, 2.0 * std::numbers::pi);
    cairo_stroke(cairo);

    // draw inner border
    Color::set_cairo_color(cairo, m_ring_border_color.get(m_state));
    cairo_set_line_width(cairo, m_ring_inner_border_width);
    cairo_arc(cairo, center_x, center_y, get_radius() - (get_thickness() / 2.0), 0, 2.0 * std::numbers::pi);
    cairo_stroke(cairo);
}

auto wall::CairoIndicatorElement::draw_highlight(cairo_t* cairo, double center_x, double center_y) -> void {
    if (!m_is_draw_ring_highlight) {
        return;
    }

    const auto highlight_start = (get_highlight_start() * (std::numbers::pi / 1024.0));
    const auto highlight_end = highlight_start + m_ring_highlight_arc;

    const auto& highlight_color = (m_highlight_state == State::Backspace) ? m_ring_highlight_color_backspace : m_ring_highlight_color_keypress;
    Color::set_cairo_color(cairo, highlight_color);
    cairo_set_line_width(cairo, get_thickness());
    cairo_arc(cairo, center_x, center_y, get_radius(), highlight_start, highlight_end);
    cairo_stroke(cairo);

    Color::set_cairo_color(cairo, m_ring_highlight_border_color);
    cairo_arc(cairo, center_x, center_y, get_radius(), highlight_start, highlight_start + m_ring_highlight_arc_border_thickness);
    cairo_stroke(cairo);

    cairo_arc(cairo, center_x, center_y, get_radius(), highlight_end, highlight_end + m_ring_highlight_arc_border_thickness);
    cairo_stroke(cairo);
}
