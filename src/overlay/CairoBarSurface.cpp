#include "overlay/CairoBarSurface.hpp"

#include <cairo.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "registry/Registry.hpp"
#include "util/Formatter.hpp"
#include "util/Log.hpp"
#include "util/NetworkDiscover.hpp"
#include "util/StringUtils.hpp"

wall::CairoBarSurface::CairoBarSurface(const Config& config, Surface* surface, wl_output_subpixel subpixel)
    : CairoSurface(config, surface, subpixel), m_bar{config}, m_battery_discover{config}, m_network_discover{config}, m_formatter{config} {
    update_settings();
    set_state(State::Idle);
}

wall::CairoBarSurface::~CairoBarSurface() = default;

auto wall::CairoBarSurface::update_settings() -> void {
    set_is_visible_on_idle(wall_conf_get(get_config(), lock_bar, visible_on_idle));
    set_idle_timeout(std::chrono::milliseconds(wall_conf_get(get_config(), lock_bar, idle_timeout_ms)));
    const auto monitor = wall_conf_get(get_config(), lock_bar, monitor);
    const auto modules_vec = StringUtils::split_and_trim(std::string{wall_conf_get(get_config(), lock_bar, modules)}, ',');
    m_is_lock_enabled = wall_conf_get(get_config(), lock_bar, enabled);
    m_is_wallpaper_enabled = wall_conf_get(get_config(), wallpaper, bar_enabled);
    m_module_separator = wall_conf_get(get_config(), lock_bar, module_separator);
    m_is_module_draw_on_empty = wall_conf_get(get_config(), lock_bar, module_draw_on_empty);

    m_is_primary_only = (monitor == "primary");

    m_modules.clear();
    m_modules.reserve(modules_vec.size());
    std::for_each(modules_vec.begin(), modules_vec.end(),
                  [this](const auto& mod) { m_modules.emplace_back(m_formatter.from_string_to_module(mod)); });

    const auto font_family = StringUtils::trim(wall_conf_get_with_fallback(get_config(), lock_bar, font, font, name));
    const auto font_size = wall_conf_get_with_fallback(get_config(), lock_bar, font_size, font, size);
    get_font_cache_mut()->load_font(font_family, font_size);

    m_bar.update_settings();
    m_last_state = StateCheck{};

    // set last activity to zero
    set_last_activity_time(std::chrono::system_clock::time_point{});
    set_last_draw_time(std::chrono::system_clock::time_point{});
}

auto wall::CairoBarSurface::generate_message() -> std::string {
    std::map<Module, std::string> replacements;

    const auto& keyboard = get_surface()->get_registry()->get_seat_mut()->get_keyboard();
    for (const auto mod : m_modules) {
        switch (mod) {
            case Module::Keyboard:
                m_keyboard_layout = keyboard.get_layout();
                replacements[mod] = m_formatter.format_keyboard(m_keyboard_layout, keyboard.is_caps_lock());
                break;
            case Module::Network:
                replacements[mod] = m_formatter.format_network(m_network_discover.get_status(get_now()));
                break;
            case Module::Battery:
                replacements[mod] = m_formatter.format_battery(m_battery_discover.get_status(get_now()));
                break;
            case Module::CapsLock:
                replacements[mod] = m_formatter.format_caps_lock(keyboard.is_caps_lock());
                break;
            case Module::Clock:
                replacements[mod] = m_formatter.format_bar_clock(get_now());
                break;
            default:
                break;
        }
    }

    return m_formatter.format(m_modules, m_module_separator, m_is_module_draw_on_empty, replacements);
}

auto wall::CairoBarSurface::should_draw() -> bool {
    if (get_surface()->get_resource_mode() == ResourceMode::Wallpaper && !m_is_wallpaper_enabled) {
        return false;
    }

    if (get_surface()->get_resource_mode() == ResourceMode::Lock && !m_is_lock_enabled) {
        return false;
    }

    if (m_is_primary_only && !get_surface()->is_primary()) {
        return false;
    }

    return should_draw_frame_on_idle(get_surface()->get_wl_bar_surface(), get_surface()->get_wl_bar_subsurface());
}

auto wall::CairoBarSurface::draw_frame(int32_t width, int32_t height) -> std::chrono::milliseconds {
    if (!should_draw()) {
        return std::chrono::milliseconds::zero();
    }

    const auto message = generate_message();
    StateCheck current_state{width, height, get_state(), message};
    if (m_last_state == current_state) {
        return std::chrono::milliseconds::zero();
    }

    const auto [buffer_width, buffer_height] = m_bar.get_buffer_size(width, height, get_font_cache(), message);
    const auto [subsurf_xpos, subsurf_ypos] = m_bar.get_position_size(width, height, buffer_width, buffer_height);

    create_cairo_surface(buffer_width, buffer_height, get_pixel_width());
    auto* cairo = get_cairo_state()->get_cairo();
    m_bar.draw(cairo, buffer_width, buffer_height, get_font_cache(), message);

    update_surface(get_surface()->get_wl_bar_surface(), get_surface()->get_wl_bar_subsurface(), subsurf_xpos, subsurf_ypos,
                   get_cairo_state()->get_buffer());
    set_last_draw_time(get_now());
    m_last_state = current_state;

    // the fast redraw time is if the clock is formatted with seconds, so min redraw time is 1 second
    return std::chrono::milliseconds{1000};
}
