#include "conf/ConfigDefaultSettings.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#include <cxxopts.hpp>
#pragma GCC diagnostic pop

#include "conf/ConfigGenerator.hpp"
#include "conf/ConfigMacros.hpp"

std::vector<std::pair<std::string, wall::conf::SettingsVariantType>> wall::conf::g_default_settings = {};  // NOLINT
std::unordered_map<std::string, std::string> wall::conf::g_description_settings = {};                      // NOLINT

void wall::conf::setup_settings() {
    wall_conf_set(color, background);
    wall_conf_set(color, foreground);
    wall_conf_set(color, 0);
    wall_conf_set(color, 1);
    wall_conf_set(color, 2);
    wall_conf_set(color, 3);
    wall_conf_set(color, 4);
    wall_conf_set(color, 5);
    wall_conf_set(color, 6);
    wall_conf_set(color, 7);
    wall_conf_set(color, 8);
    wall_conf_set(color, 9);
    wall_conf_set(color, 10);
    wall_conf_set(color, 11);
    wall_conf_set(color, 12);
    wall_conf_set(color, 13);
    wall_conf_set(color, 14);
    wall_conf_set(color, 15);
    wall_conf_set(config, file);

    wall_conf_set(color_scheme, file);
    wall_conf_set(debug, mode);
    wall_conf_set(command, ignore_is_running);
    wall_conf_set(command, name);
    wall_conf_set(command, socket_backlog);
    wall_conf_set(command, socket_filename);
    wall_conf_set(start, lock);
    wall_conf_set(password, allow_empty);
    wall_conf_set(password, max_length);
    wall_conf_set(font, name);
    wall_conf_set(font, size);
    wall_conf_set(font, color);
    wall_conf_set(background, color);
    wall_conf_set(border, color);
    wall_conf_set(border, width);
    wall_conf_set(monitor, primary);
    wall_conf_set(grace, period_secs);
    wall_conf_set(input_inhibitor, enforce);
    wall_conf_set(general, daemonize);
    wall_conf_set(general, force_software_rendering);

    wall_conf_set(file, path);
    wall_conf_set(file, extensions);
    wall_conf_set(file, mute);
    wall_conf_set(file, fit);
    wall_conf_set(file, sort_order);
    wall_conf_set(file, loop);
    wall_conf_set(file, keep_same_order);
    wall_conf_set(file, image_change_interval_secs);
    wall_conf_set(file, video_preload_secs);
    wall_conf_set(file, video_max_change_interval_secs);
    wall_conf_set(file, screenshot_enabled);
    wall_conf_set(file, screenshot_cache_enabled);
    wall_conf_set(file, screenshot_directory);
    wall_conf_set(file, screenshot_delay_ms);
    wall_conf_set(file, screenshot_done_cmd);
    wall_conf_set(file, screenshot_format);
    wall_conf_set(file, screenshot_filename);
    wall_conf_set(file, screenshot_reload_on_done);

    wall_conf_set(wallpaper, enabled);
    wall_conf_set(wallpaper, pause_after_unlock);
    wall_conf_set(wallpaper, dismiss_after_pause);
    wall_conf_set(wallpaper, pause_after_unlock_delay_secs);
    wall_conf_set(wallpaper, path);
    wall_conf_set(wallpaper, extensions);
    wall_conf_set(wallpaper, mute);
    wall_conf_set(wallpaper, fit);
    wall_conf_set(wallpaper, sort_order);
    wall_conf_set(wallpaper, loop);
    wall_conf_set(wallpaper, keep_same_order);
    wall_conf_set(wallpaper, image_change_interval_secs);
    wall_conf_set(wallpaper, video_preload_secs);
    wall_conf_set(wallpaper, video_max_change_interval_secs);
    wall_conf_set(wallpaper, screenshot_enabled);
    wall_conf_set(wallpaper, screenshot_cache_enabled);
    wall_conf_set(wallpaper, screenshot_directory);
    wall_conf_set(wallpaper, screenshot_delay_ms);
    wall_conf_set(wallpaper, screenshot_done_cmd);
    wall_conf_set(wallpaper, bar_enabled);
    wall_conf_set(wallpaper, indicator_enabled);
    wall_conf_set(wallpaper, screenshot_reload_on_done);

    wall_conf_set(lock, path);
    wall_conf_set(lock, extensions);
    wall_conf_set(lock, mute);
    wall_conf_set(lock, fit);
    wall_conf_set(lock, sort_order);
    wall_conf_set(lock, loop);
    wall_conf_set(lock, keep_same_order);
    wall_conf_set(lock, image_change_interval_secs);
    wall_conf_set(lock, video_preload_secs);
    wall_conf_set(lock, video_max_change_interval_secs);
    wall_conf_set(lock, screenshot_enabled);
    wall_conf_set(lock, screenshot_cache_enabled);
    wall_conf_set(lock, screenshot_directory);
    wall_conf_set(lock, screenshot_delay_ms);
    wall_conf_set(lock, screenshot_done_cmd);
    wall_conf_set(lock, screenshot_reload_on_done);

    wall_conf_set(lock_indicator, enabled);
    wall_conf_set(lock_indicator, monitor);
    wall_conf_set(lock_indicator, image_path);
    wall_conf_set(lock_indicator, visible_on_idle);
    wall_conf_set(lock_indicator, idle_timeout_ms);
    wall_conf_set(lock_indicator, clock_enabled);
    wall_conf_set(lock_indicator, clock_format);
    wall_conf_set(lock_indicator, ring_enabled);
    wall_conf_set(lock_indicator, ring_radius);
    wall_conf_set(lock_indicator, ring_thickness);
    wall_conf_set(lock_indicator, ring_fill_color_input);
    wall_conf_set(lock_indicator, ring_fill_color_cleared);
    wall_conf_set(lock_indicator, ring_fill_color_caps_lock);
    wall_conf_set(lock_indicator, ring_fill_color_verifying);
    wall_conf_set(lock_indicator, ring_fill_color_wrong);
    wall_conf_set(lock_indicator, ring_inner_enabled);
    wall_conf_set(lock_indicator, ring_inner_fill_color_input);
    wall_conf_set(lock_indicator, ring_inner_fill_color_cleared);
    wall_conf_set(lock_indicator, ring_inner_fill_color_caps_lock);
    wall_conf_set(lock_indicator, ring_inner_fill_color_verifying);
    wall_conf_set(lock_indicator, ring_inner_fill_color_wrong);
    wall_conf_set(lock_indicator, ring_border_color_input);
    wall_conf_set(lock_indicator, ring_border_color_cleared);
    wall_conf_set(lock_indicator, ring_border_color_caps_lock);
    wall_conf_set(lock_indicator, ring_border_color_verifying);
    wall_conf_set(lock_indicator, ring_border_color_wrong);
    wall_conf_set(lock_indicator, ring_border_width);
    wall_conf_set(lock_indicator, ring_inner_border_width);
    wall_conf_set(lock_indicator, ring_highlight_color_keypress);
    wall_conf_set(lock_indicator, ring_highlight_color_backspace);
    wall_conf_set(lock_indicator, ring_highlight_arc);
    wall_conf_set(lock_indicator, ring_highlight_arc_thickness);
    wall_conf_set(lock_indicator, ring_highlight_arc_border_thickness);
    wall_conf_set(lock_indicator, ring_highlight_border_color);
    wall_conf_set(lock_indicator, font);
    wall_conf_set(lock_indicator, font_size);
    wall_conf_set(lock_indicator, font_color_input);
    wall_conf_set(lock_indicator, font_color_cleared);
    wall_conf_set(lock_indicator, font_color_caps_lock);
    wall_conf_set(lock_indicator, font_color_verifying);
    wall_conf_set(lock_indicator, font_color_wrong);
    wall_conf_set(lock_indicator, message_input);
    wall_conf_set(lock_indicator, message_cleared);
    wall_conf_set(lock_indicator, message_caps_lock);
    wall_conf_set(lock_indicator, message_verifying);
    wall_conf_set(lock_indicator, message_wrong);

    wall_conf_set(lock_indicator, analog_clock_enabled);
    wall_conf_set(lock_indicator, analog_clock_hour_hand_length);
    wall_conf_set(lock_indicator, analog_clock_minute_hand_length);
    wall_conf_set(lock_indicator, analog_clock_second_hand_length);
    wall_conf_set(lock_indicator, analog_clock_hour_hand_thickness);
    wall_conf_set(lock_indicator, analog_clock_minute_hand_thickness);
    wall_conf_set(lock_indicator, analog_clock_second_hand_thickness);
    wall_conf_set(lock_indicator, analog_clock_center_radius);
    wall_conf_set(lock_indicator, analog_clock_hour_marker_radius);
    wall_conf_set(lock_indicator, analog_clock_hour_marker_length);
    wall_conf_set(lock_indicator, analog_clock_hour_marker_thickness);
    wall_conf_set(lock_indicator, analog_clock_second_marker_radius);
    wall_conf_set(lock_indicator, analog_clock_second_marker_length);
    wall_conf_set(lock_indicator, analog_clock_second_marker_thickness);
    wall_conf_set(lock_indicator, analog_clock_hour_hand_enabled);
    wall_conf_set(lock_indicator, analog_clock_minute_hand_enabled);
    wall_conf_set(lock_indicator, analog_clock_second_hand_enabled);
    wall_conf_set(lock_indicator, analog_clock_center_enabled);
    wall_conf_set(lock_indicator, analog_clock_hour_marker_enabled);
    wall_conf_set(lock_indicator, analog_clock_second_marker_enabled);
    wall_conf_set(lock_indicator, analog_clock_hand_color_input);
    wall_conf_set(lock_indicator, analog_clock_hand_color_cleared);
    wall_conf_set(lock_indicator, analog_clock_hand_color_caps_lock);
    wall_conf_set(lock_indicator, analog_clock_hand_color_verifying);
    wall_conf_set(lock_indicator, analog_clock_hand_color_wrong);
    wall_conf_set(lock_indicator, analog_clock_center_color_input);
    wall_conf_set(lock_indicator, analog_clock_center_color_cleared);
    wall_conf_set(lock_indicator, analog_clock_center_color_caps_lock);
    wall_conf_set(lock_indicator, analog_clock_center_color_verifying);
    wall_conf_set(lock_indicator, analog_clock_center_color_wrong);
    wall_conf_set(lock_indicator, analog_clock_marker_color_input);
    wall_conf_set(lock_indicator, analog_clock_marker_color_cleared);
    wall_conf_set(lock_indicator, analog_clock_marker_color_caps_lock);
    wall_conf_set(lock_indicator, analog_clock_marker_color_verifying);
    wall_conf_set(lock_indicator, analog_clock_marker_color_wrong);

    wall_conf_set(lock_bar, enabled);
    wall_conf_set(lock_bar, monitor);
    wall_conf_set(lock_bar, visible_on_idle);
    wall_conf_set(lock_bar, idle_timeout_ms);
    wall_conf_set(lock_bar, modules);
    wall_conf_set(lock_bar, module_separator);
    wall_conf_set(lock_bar, module_draw_on_empty);
    wall_conf_set(lock_bar, font);
    wall_conf_set(lock_bar, font_size);
    wall_conf_set(lock_bar, font_color);
    wall_conf_set(lock_bar, background_color);
    wall_conf_set(lock_bar, border_color);
    wall_conf_set(lock_bar, border_width);
    wall_conf_set(lock_bar, corner_radius);
    wall_conf_set(lock_bar, alignment);
    wall_conf_set(lock_bar, left_padding);
    wall_conf_set(lock_bar, right_padding);
    wall_conf_set(lock_bar, top_padding);
    wall_conf_set(lock_bar, bottom_padding);
    wall_conf_set(lock_bar, text_top_bottom_margin);

    wall_conf_set(lock_bar, keyboard_format);
    wall_conf_set(lock_bar, layout_guess_short_name);

    wall_conf_set(lock_bar, network_format_wifi);
    wall_conf_set(lock_bar, network_format_ethernet);
    wall_conf_set(lock_bar, network_format_disconnected);
    wall_conf_set(lock_bar, network_format_ifname_max_length);
    wall_conf_set(lock_bar, network_update_interval_secs);

    wall_conf_set(lock_bar, caps_locked_format);
    wall_conf_set(lock_bar, caps_unlocked_format);

    wall_conf_set(lock_bar, battery_level_icons);
    wall_conf_set(lock_bar, battery_not_found);
    wall_conf_set(lock_bar, battery_plugged_format);
    wall_conf_set(lock_bar, battery_charging_format);
    wall_conf_set(lock_bar, battery_discharging_format);
    wall_conf_set(lock_bar, battery_font_color);
    wall_conf_set(lock_bar, battery_update_interval_secs);

    wall_conf_set(lock_bar, clock_format);

    wall_conf_set(log, level);
    wall_conf_set(log, threads);
    wall_conf_set(log, line_count);
    wall_conf_set(log, file_size);
    wall_conf_set(log, file);
}

auto wall::conf::print_default_config(std::ostream& stream) -> void {
    ConfigGenerator::generate_example(stream, g_default_settings, g_description_settings);
}

auto wall::conf::print_default_config_markdown(std::ostream& stream) -> void {
    ConfigGenerator::generate_example_markdown(stream, g_default_settings, g_description_settings);
}
