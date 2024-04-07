#include "overlay/CairoIndicatorMessage.hpp"
#include <chrono>
#include "util/Formatter.hpp"

wall::CairoIndicatorMessage::CairoIndicatorMessage(const Config& config) : m_config{config} { update_settings(); }

auto wall::CairoIndicatorMessage::get_config() const -> const Config& { return m_config; }

auto wall::CairoIndicatorMessage::get_message() const -> const std::string& { return m_message; }

auto wall::CairoIndicatorMessage::update_settings() -> void {
    m_is_clock_enabled = wall_conf_get(get_config(), lock_indicator, clock_enabled);
    m_message_input = wall_conf_get(get_config(), lock_indicator, message_input);
    m_message_cleared = wall_conf_get(get_config(), lock_indicator, message_cleared);
    m_message_caps_lock = wall_conf_get(get_config(), lock_indicator, message_caps_lock);
    m_message_verifying = wall_conf_get(get_config(), lock_indicator, message_verifying);
    m_message_wrong = wall_conf_get(get_config(), lock_indicator, message_wrong);

    const auto is_analog_clock_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_enabled);
    if (is_analog_clock_enabled) {
        if (get_config().is_set_from_config_file(conf::k_lock_indicator_clock_enabled) &&
            get_config().is_set_from_config_file(conf::k_lock_indicator_analog_clock_enabled)) {
            LOG_WARN("Both lock_clock_enabled and lock_analog_clock_enabled are set in the config file. lock_analog_clock_enabled will be ignored.");
        } else if (!get_config().is_set_from_config_file(conf::k_lock_indicator_clock_enabled) &&
                   get_config().is_set_from_config_file(conf::k_lock_indicator_analog_clock_enabled)) {
            LOG_DEBUG("Disabling digital clock as analog clock is enabled.");
            m_is_clock_enabled = false;
        }
    }

    m_font_color.m_input = wall_conf_get_with_fallback(get_config(), lock_indicator, font_color_input, font, color);
    m_font_color.m_cleared = wall_conf_get(get_config(), lock_indicator, font_color_cleared);
    m_font_color.m_caps_lock = wall_conf_get_with_fallback(get_config(), lock_indicator, font_color_caps_lock, font, color);
    m_font_color.m_verifying = wall_conf_get(get_config(), lock_indicator, font_color_verifying);
    m_font_color.m_wrong = wall_conf_get(get_config(), lock_indicator, font_color_wrong);
}

auto wall::CairoIndicatorMessage::update_message(State state, std::chrono::time_point<std::chrono::system_clock> now) -> void {
    m_message = get_message_format(state);

    // only show clock if there is no message set
    if (m_message.empty() && m_is_clock_enabled) {
        Formatter fmter{get_config()};
        m_message = fmter.format_indicator_clock(now);
    }
}

auto wall::CairoIndicatorMessage::get_message_format(State state) const -> const std::string& {
    switch (state) {
        case State::Input:
            return m_message_input;
        case State::Cleared:
            return m_message_cleared;
        case State::CapsLock:
            return m_message_caps_lock;
        case State::Verifying:
            return m_message_verifying;
        case State::Wrong:
            return m_message_wrong;
        default:
            return m_message_input;
    }
}

auto wall::CairoIndicatorMessage::draw(cairo_t* cairo, State state, const CairoFontCache& font_cache, double x_pos, double y_pos)
    -> std::chrono::milliseconds {
    if (m_message.empty()) {
        return std::chrono::milliseconds::zero();
    }

    cairo_set_font_face(cairo, font_cache.get_font_face());
    cairo_set_font_size(cairo, font_cache.get_font_size());

    Color::set_cairo_color(cairo, m_font_color.get(state));

    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_text_extents(cairo, m_message.c_str(), &text_extents);
    cairo_font_extents(cairo, &font_extents);
    auto text_x = x_pos - (text_extents.width / 2.0) + text_extents.x_bearing;
    auto text_y = y_pos + (font_extents.height / 2.0) - font_extents.descent;
    cairo_move_to(cairo, text_x, text_y);
    cairo_show_text(cairo, m_message.c_str());

    return m_is_clock_enabled ? std::chrono::milliseconds{1000} : std::chrono::milliseconds::zero();
}

auto wall::CairoIndicatorMessage::get_text_width(const CairoFontCache& font_cache) const -> double {
    auto* cairo = font_cache.get_font_cairo_state()->get_cairo();
    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    cairo_set_font_face(cairo, font_cache.get_font_face());
    cairo_set_font_size(cairo, font_cache.get_font_size());

    if (!m_message.empty()) {
        cairo_text_extents_t text_extents;

        cairo_text_extents(cairo, m_message.c_str(), &text_extents);
        return text_extents.width;
    }

    return 0.0;
}
