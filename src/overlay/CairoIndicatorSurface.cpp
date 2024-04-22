#include "overlay/CairoIndicatorSurface.hpp"

#include <cairo.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <chrono>
#include "registry/Registry.hpp"
#include "util/Log.hpp"
#include "util/StringUtils.hpp"

wall::CairoIndicatorSurface::CairoIndicatorSurface(const Config& config, Surface* surface, wl_output_subpixel subpixel)
    : CairoSurface(config, surface, subpixel), m_indicator{config}, m_indicator_message{config}, m_analog_clock{config} {
    update_settings();

    m_last_state = StateCheck{};
    set_state(State::Idle);
}

wall::CairoIndicatorSurface::~CairoIndicatorSurface() = default;

auto wall::CairoIndicatorSurface::update_settings() -> void {
    m_is_lock_enabled = wall_conf_get(get_config(), lock_indicator, enabled);
    m_is_wallpaper_enabled = wall_conf_get(get_config(), wallpaper, indicator_enabled);
    const auto monitor = wall_conf_get(get_config(), lock_indicator, monitor);
    m_is_analog_clock_enabled = wall_conf_get(get_config(), lock_indicator, analog_clock_enabled);
    set_is_visible_on_idle(wall_conf_get(get_config(), lock_indicator, visible_on_idle));
    set_idle_timeout(std::chrono::milliseconds(wall_conf_get(get_config(), lock_indicator, idle_timeout_ms)));

    m_is_primary_only = (monitor == "primary");

    const auto font_family = StringUtils::trim(wall_conf_get_with_fallback(get_config(), lock_indicator, font, font, name));
    const auto font_size = (get_font_cache().get_font_size() <= 0.0) ? m_indicator.get_radius() / 3.0 : get_font_cache().get_font_size();
    get_font_cache_mut()->load_font(font_family, font_size);

    m_indicator.update_settings();
    m_indicator_message.update_settings();
    m_analog_clock.update_settings();
    m_last_state = StateCheck{};

    // set last activity to zero
    set_last_activity_time(std::chrono::system_clock::time_point{});
    set_last_draw_time(std::chrono::system_clock::time_point{});
}

auto wall::CairoIndicatorSurface::update_message() -> void {
    const auto& keyboard = get_surface()->get_registry()->get_seat_mut()->get_keyboard();
    if (keyboard.is_caps_lock() && (get_state() == State::Keypress || get_state() == State::Backspace || get_state() == State::Input)) {
        set_state(State::CapsLock);
    } else if (!keyboard.is_caps_lock() && get_state() == State::CapsLock) {
        set_state(State::Input);
    }

    m_indicator_message.update_message(get_state(), get_now());
}

auto wall::CairoIndicatorSurface::on_state_change(State state) -> void {
    CairoSurface::on_state_change(state);
    m_indicator.on_state_change(state);
}

auto wall::CairoIndicatorSurface::get_indicator_message() -> const CairoIndicatorMessage& { return m_indicator_message; }

auto wall::CairoIndicatorSurface::should_draw() -> bool {
    if (get_surface()->get_resource_mode() == ResourceMode::Wallpaper && !m_is_wallpaper_enabled) {
        return false;
    }

    if (get_surface()->get_resource_mode() == ResourceMode::Lock && !m_is_lock_enabled) {
        return false;
    }

    if (m_is_primary_only && !get_surface()->is_primary()) {
        return false;
    }

    if (!should_draw_frame_on_idle(get_surface()->get_wl_indicator_surface(), get_surface()->get_wl_indicator_subsurface())) {
        return false;
    }

    if (m_is_analog_clock_enabled &&
        m_analog_clock.should_redraw(std::chrono::duration_cast<std::chrono::milliseconds>(get_now() - get_last_draw_time()))) {
        // force redraw if analog clock needs to be updated
        m_last_state = StateCheck{};
        return true;
    }

    return true;
}

auto wall::CairoIndicatorSurface::draw_frame(int32_t width, int32_t height) -> std::chrono::milliseconds {
    if (!should_draw()) {
        return std::chrono::milliseconds::zero();
    }

    update_message();
    StateCheck current_state{width, height, m_indicator.get_highlight_start(), get_state(), m_indicator_message.get_message()};
    if (m_last_state == current_state) {
        return std::chrono::milliseconds::zero();
    }

    const auto radius = m_indicator.get_radius();
    const auto thickness = m_indicator.get_thickness();
    const auto buffer_diameter = (radius + thickness) * 2;

    const auto [buffer_width, buffer_height] = get_buffer_size(buffer_diameter);
    if (buffer_width == 0 || buffer_height == 0 || buffer_width > width || buffer_height > height) {
        LOG_FATAL("Invalid buffer size for indicator");
    }

    create_cairo_surface(buffer_width, buffer_height, get_pixel_width());
    auto* cairo = get_cairo_state()->get_cairo();

    const auto center_x = buffer_width / 2.0;
    const auto center_y = buffer_height / 2.0;

    const auto min_indicator_redraw_time = m_indicator.draw(cairo, center_x, center_y);
    auto min_msg_redraw_time = m_indicator_message.draw(cairo, get_state(), get_font_cache(), buffer_width / 2.0, buffer_height / 2.0);
    if (m_indicator_message.get_message().empty() && m_is_analog_clock_enabled) {
        min_msg_redraw_time = m_analog_clock.draw(cairo, center_x, center_y, get_state());
    }

    auto min_redraw_time = min_indicator_redraw_time;
    if (min_redraw_time.count() == 0) {
        min_redraw_time = min_msg_redraw_time;
    } else if (min_msg_redraw_time.count() > 0) {
        min_redraw_time = std::min(min_redraw_time, min_msg_redraw_time);
    }

    const auto subsurf_xpos = static_cast<int32_t>((width / 2.0) - (buffer_width / 2.0) + 2.0);
    const auto subsurf_ypos = static_cast<int32_t>((height / 2.0) - (radius + thickness));

    update_surface(get_surface()->get_wl_indicator_surface(), get_surface()->get_wl_indicator_subsurface(), subsurf_xpos, subsurf_ypos,
                   get_cairo_state()->get_buffer());
    m_last_state = current_state;
    set_last_draw_time(get_now());
    return min_redraw_time;
}

auto wall::CairoIndicatorSurface::get_buffer_size(int32_t buffer_diameter) -> std::pair<int32_t, int32_t> {
    auto buffer_width = buffer_diameter;
    auto buffer_height = buffer_diameter;

    resize_based_on_text(&buffer_width);

    return {buffer_width, buffer_height};
}

auto wall::CairoIndicatorSurface::resize_based_on_text(int32_t* width) -> void {
    const auto text_width = m_indicator_message.get_text_width(get_font_cache());

    if (*width < text_width) {
        *width = static_cast<int32_t>(text_width);
    }
}
