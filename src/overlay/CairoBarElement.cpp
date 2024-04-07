#include "overlay/CairoBarElement.hpp"

#include <numbers>

#include "conf/ConfigMacros.hpp"
#include "overlay/CairoFontCache.hpp"
#include "overlay/CairoState.hpp"
#include "util/StringUtils.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::CairoBarElement::CairoBarElement(const Config& config) : m_config(config) { update_settings(); }

auto wall::CairoBarElement::get_config() const -> const Config& { return m_config; }

auto wall::CairoBarElement::alignment_from_string(std::string_view str_orig) const -> BarAlignment {
    const auto str = StringUtils::trim(str_orig);
    if (str == "top-left") {
        return BarAlignment::TopLeft;
    }

    if (str == "top-center") {
        return BarAlignment::TopCenter;
    }

    if (str == "top-right") {
        return BarAlignment::TopRight;
    }

    if (str == "bottom-right") {
        return BarAlignment::BottomRight;
    }

    if (str == "bottom-center") {
        return BarAlignment::BottomCenter;
    }

    if (str == "bottom-left") {
        return BarAlignment::BottomLeft;
    }
    return BarAlignment::None;
}

auto wall::CairoBarElement::update_settings() -> void {
    m_background_color = wall_conf_get_with_fallback(get_config(), lock_bar, background_color, background, color);
    m_border_color = wall_conf_get_with_fallback(get_config(), lock_bar, border_color, border, color);
    m_border_width = wall_conf_get_with_fallback(get_config(), lock_bar, border_width, border, width);
    m_corner_radius = wall_conf_get(get_config(), lock_bar, corner_radius);

    m_alignment = alignment_from_string(wall_conf_get(get_config(), lock_bar, alignment));
    m_left_padding = wall_conf_get(get_config(), lock_bar, left_padding);
    m_right_padding = wall_conf_get(get_config(), lock_bar, right_padding);
    m_top_padding = wall_conf_get(get_config(), lock_bar, top_padding);
    m_bottom_padding = wall_conf_get(get_config(), lock_bar, bottom_padding);
    m_text_top_bottom_margin = wall_conf_get(get_config(), lock_bar, text_top_bottom_margin);

    m_font_color = wall_conf_get_with_fallback(get_config(), lock_bar, font_color, font, color);
}

auto wall::CairoBarElement::get_buffer_size(int32_t width,
                                            int32_t height,
                                            int32_t scale,
                                            const CairoFontCache& font_cache,
                                            const std::string& message) const -> std::pair<int32_t, int32_t> {
    auto* font_face = font_cache.get_font_face();
    auto font_size = font_cache.get_font_size();
    auto* cairo = font_cache.get_font_cairo_state()->get_cairo();
    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
    cairo_set_font_face(cairo, font_face);
    cairo_set_font_size(cairo, font_size);

    auto buffer_width = width;
    auto buffer_height = height;

    if (!message.empty()) {
        cairo_text_extents_t text_extents;

        cairo_text_extents(cairo, message.c_str(), &text_extents);
        buffer_width = (text_extents.width < buffer_width) ? text_extents.width : buffer_width;
        buffer_height = (text_extents.height < buffer_height) ? text_extents.height : buffer_height;
        buffer_width += (m_border_width * 2);
        buffer_height += (m_border_width * 2) + (2 * m_text_top_bottom_margin);

        const auto corner_radius = m_corner_radius == 0.0 ? buffer_height / 2.0 : m_corner_radius;
        buffer_width += 2 * corner_radius;
    }

    if (scale > 1) {
        // Ensure buffer size is multiple of buffer scale - required by protocol
        buffer_width += scale - (buffer_width % scale);
        buffer_height += scale - (buffer_height % scale);
    }

    return {buffer_width, buffer_height};
}

auto wall::CairoBarElement::get_position_size(int32_t width, int32_t height, int32_t buffer_width, int32_t buffer_height) const
    -> std::pair<int32_t, int32_t> {
    auto subsurf_xpos = 0;
    auto subsurf_ypos = 0;

    switch (m_alignment) {
        case BarAlignment::TopLeft:
            subsurf_xpos = m_left_padding;
            subsurf_ypos = m_top_padding;
            break;
        case BarAlignment::TopCenter:
            subsurf_xpos = ((width - buffer_width) / 2);
            subsurf_ypos = m_top_padding;
            break;
        case BarAlignment::TopRight:
            subsurf_xpos = (width - buffer_width) - m_right_padding;
            subsurf_ypos = m_top_padding;
            break;
        case BarAlignment::BottomRight:
            subsurf_xpos = (width - buffer_width) - m_right_padding;
            subsurf_ypos = (height - buffer_height) - m_bottom_padding;
            break;
        case BarAlignment::BottomCenter:
            subsurf_xpos = ((width - buffer_width) / 2);
            subsurf_ypos = (height - buffer_height) - m_bottom_padding;
            break;
        case BarAlignment::BottomLeft:
            subsurf_xpos = m_left_padding;
            subsurf_ypos = (height - buffer_height) - m_bottom_padding;
            break;
        case BarAlignment::None:
            break;
    }

    return {subsurf_xpos, subsurf_ypos};
}

auto wall::CairoBarElement::draw(cairo_t* cairo, double width, double height, const CairoFontCache& font_cache, const std::string& message) const
    -> void {
    draw_background(cairo, width, height);
    draw_text(cairo, width, height, font_cache, message);
}

auto wall::CairoBarElement::draw_text(cairo_t* cairo, double width, double height, const CairoFontCache& font_cache, const std::string& message) const
    -> void {
    Color::set_cairo_color(cairo, m_font_color);
    cairo_set_font_face(cairo, font_cache.get_font_face());
    cairo_set_font_size(cairo, font_cache.get_font_size());

    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_text_extents(cairo, message.c_str(), &text_extents);
    cairo_font_extents(cairo, &font_extents);
    const auto text_x = (width / 2.0) - (text_extents.width / 2.0) + text_extents.x_bearing;
    const auto text_y = (height / 2.0) + (font_extents.height / 2.0) - font_extents.descent;

    cairo_move_to(cairo, text_x, text_y);
    cairo_show_text(cairo, message.c_str());
}

auto wall::CairoBarElement::draw_background(cairo_t* cairo, double width, double height) const -> void {
    const auto corner_radius = m_corner_radius == 0.0 ? height / 2.0 : m_corner_radius;
    rounded_rectangle(cairo, 0.0, 0.0, width, height, corner_radius);
}

void wall::CairoBarElement::rounded_rectangle(cairo_t* cairo, double x_pos, double y_pos, double width, double height, double corner_radius_1) const {
    const auto aspect = 1.0;  // aspect ratio
    const auto corner_radius = corner_radius_1 / aspect;
    const auto degrees = std::numbers::pi / 180.0;

    cairo_new_sub_path(cairo);
    cairo_arc(cairo, x_pos + width - corner_radius, y_pos + corner_radius, corner_radius, -90.0 * degrees, 0.0 * degrees);
    cairo_arc(cairo, x_pos + width - corner_radius, y_pos + height - corner_radius, corner_radius, 0.0 * degrees, 90.0 * degrees);
    cairo_arc(cairo, x_pos + corner_radius, y_pos + height - corner_radius, corner_radius, 90.0 * degrees, 180.0 * degrees);
    cairo_arc(cairo, x_pos + corner_radius, y_pos + corner_radius, corner_radius, 180.0 * degrees, 270.0 * degrees);
    cairo_close_path(cairo);

    Color::set_cairo_color(cairo, m_background_color);
    cairo_fill_preserve(cairo);
    Color::set_cairo_color(cairo, m_border_color);
    cairo_set_line_width(cairo, m_border_width);
    cairo_stroke(cairo);
}
