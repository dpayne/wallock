#pragma once

#include <cmath>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "conf/ConfigMacros.hpp"

namespace wall::conf {
class OptionsMap;

using SettingsVariantType = std::variant<unsigned long, long, bool, double, std::string>;
extern std::vector<std::pair<std::string, SettingsVariantType>> g_default_settings;  // NOLINT
extern std::unordered_map<std::string, std::string> g_description_settings;          // NOLINT

// clang-format off

// default color scheme
wall_conf_key(color, background, "#000000", "Default color scheme.")
wall_conf_key(color, foreground, "#FFFFFF", "")
wall_conf_key(color, 0, "#000000", "")
wall_conf_key(color, 1, "#337D00", "")
wall_conf_key(color, 2, "#33DB00", "")
wall_conf_key(color, 3, "#DB3300", "")
wall_conf_key(color, 4, "#000000", "")
wall_conf_key(color, 5, "#000000", "")
wall_conf_key(color, 6, "#000000", "")
wall_conf_key(color, 7, "#000000", "")
wall_conf_key(color, 8, "#E5A445", "")
wall_conf_key(color, 9, "#3300FF", "")
wall_conf_key(color, 10, "#FFFFFF", "")
wall_conf_key(color, 11, "#000000", "")
wall_conf_key(color, 12, "#000000", "")
wall_conf_key(color, 13, "#000000", "")
wall_conf_key(color, 14, "#000000", "")
wall_conf_key(color, 15, "#000000", "")


// config settings
wall_conf_key(config, file, "config", "Configuration file path.")
wall_conf_key(color_scheme, file, "color_scheme", "Configuration file path.")
wall_conf_key(debug, mode, false, "Enables debugging options.")
wall_conf_key(command, ignore_is_running, false, "Ignores if another instance is running.")
wall_conf_key(command, name, "", "")
wall_conf_key(start, lock, false, "Starts lock immediately on startup.")
wall_conf_key(password, allow_empty, false, "Allow empty passwords.")
wall_conf_key(password, max_length, 1024UL, "Maximum password length.")
wall_conf_key(font, name, "FiraCode Nerd Font", "Default font name.")
wall_conf_key(font, size, 20.0, "Default font size.")
wall_conf_key(font, color, "{color10}FF", "Default font color.")
wall_conf_key(background, color, "{background}C0", "Default background color.")
wall_conf_key(border, color, "{color6}FF", "Default border color.")
wall_conf_key(border, width, 2, "Default border width.")
wall_conf_key(monitor, primary, "", "Set primary monitor")
wall_conf_key(grace, period_secs, 2, "Grace period in seconds for disabling password authentication.")
wall_conf_key(general, daemonize, true, "Daemonize, run in background.")
wall_conf_key(general, force_software_rendering, false, "Force software rendering.")
wall_conf_key(general, mpv_logging_enabled, false, "Enable mpv logging.")
wall_conf_key(general, lock_cmd, "", "Command to run after locking, the process will be terminated after the lock screen is dismissed.")
wall_conf_key(command, socket_backlog, 128, "Number of connections to allow in the socket backlog.")
wall_conf_key(command, socket_filename, "wallock.sock", "Socket filename.")

// file settings
wall_conf_key(file, path, "~/.wallpapers/", "Path for wallpaper and lock screen resources.")
wall_conf_key(file, extensions, "jpg,jpeg,png,mp4,mov,webm,avi,wmv,flv,mkv", "Allowed extensions for wallpaper and lock screen resources.")
wall_conf_key(file, mute, true, "Enabling or disabling muting of resources.")
wall_conf_key(file, fit, "fill", "Options are fit or fill.")
wall_conf_key(file, sort_order, "random", "Sort order for resources, options are random or alpha.")
wall_conf_key(file, loop, false, "Loop first loaded resource.")
wall_conf_key(file, keep_same_order, false, "Enforces the same ordering of resources between multiple monitors.")
wall_conf_key(file, image_change_interval_secs, 900UL, "How long to display an image for before rotating.")
wall_conf_key(file, video_preload_secs, 1UL, "How long before the current resource finishes before loading the next. Note this may cutoff the end of a video.")
wall_conf_key(file, video_max_change_interval_secs, 0UL, "Maximum time in seconds a video is allowed to play before rotating, default is unlimited.")
wall_conf_key(file, screenshot_enabled, false, "Enables taking a screenshot after a new resource has been loaded.")
wall_conf_key(file, screenshot_cache_enabled, true, "Caches the screenshot of a video.")
wall_conf_key(file, screenshot_directory, "", "Screenshot location, default will be ~/.cache/wallock.")
wall_conf_key(file, screenshot_delay_ms, 1000UL, "How long to wait until after a resource loads to take a screenshot.")
wall_conf_key(file, screenshot_done_cmd, "", "Command to run after a screenshot has been taken.")
wall_conf_key(file, screenshot_format, "jpg", "Screenshot format, note png can be very slow and large, jpg is recommended.")
wall_conf_key(file, screenshot_filename, "{filename}.{format}", "Screenshot filename.")
wall_conf_key(file, screenshot_reload_on_done, true, "Reload colors on success.")


// wallpaper settings
wall_conf_key(wallpaper, enabled, true, "Enables wallpaper.")
wall_conf_key(wallpaper, pause_after_unlock, false, "Pauses the wallpaper after unlocking.")
wall_conf_key(wallpaper, pause_after_unlock_delay_secs, 10, "Delay in seconds before pausing the wallpaper after unlocking.")
wall_conf_key(wallpaper, dismiss_after_pause, false, "Dismisses the wallpaper after pausing.")
wall_conf_key(wallpaper, path, "", "These are optional and override the file settings for the wallpaper.")
wall_conf_key(wallpaper, extensions,k_default_file_extensions, "")
wall_conf_key(wallpaper, mute, k_default_file_mute, "")
wall_conf_key(wallpaper, fit, k_default_file_fit, "")
wall_conf_key(wallpaper, sort_order, k_default_file_sort_order, "")
wall_conf_key(wallpaper, loop, false, "Loop first loaded resource.")
wall_conf_key(wallpaper, keep_same_order, k_default_file_keep_same_order, "")
wall_conf_key(wallpaper, image_change_interval_secs, k_default_file_image_change_interval_secs, "")
wall_conf_key(wallpaper, video_preload_secs, k_default_file_video_preload_secs, "")
wall_conf_key(wallpaper, video_max_change_interval_secs, k_default_file_video_max_change_interval_secs, "")
wall_conf_key(wallpaper, screenshot_enabled, k_default_file_screenshot_enabled, "")
wall_conf_key(wallpaper, screenshot_cache_enabled, k_default_file_screenshot_cache_enabled, "")
wall_conf_key(wallpaper, screenshot_directory, k_default_file_screenshot_directory, "")
wall_conf_key(wallpaper, screenshot_delay_ms, k_default_file_screenshot_delay_ms, "")
wall_conf_key(wallpaper, screenshot_done_cmd, "", "")
wall_conf_key(wallpaper, bar_enabled, false, "")
wall_conf_key(wallpaper, indicator_enabled, false, "")
wall_conf_key(wallpaper, screenshot_reload_on_done, true, "Reload colors on success.")

// these are optional and override the file settings for the lock screen
wall_conf_key(lock, path, "", "These are optional and override the file settings for the lock screen.")
wall_conf_key(lock, extensions, k_default_file_extensions, "")
wall_conf_key(lock, mute, k_default_file_mute, "")
wall_conf_key(lock, fit, k_default_file_fit, "")
wall_conf_key(lock, sort_order, k_default_file_sort_order, "")
wall_conf_key(lock, loop, false, "Loop first loaded resource.")
wall_conf_key(lock, keep_same_order, k_default_file_keep_same_order, "")
wall_conf_key(lock, image_change_interval_secs, k_default_file_image_change_interval_secs, "")
wall_conf_key(lock, video_preload_secs, k_default_file_video_preload_secs, "")
wall_conf_key(lock, video_max_change_interval_secs, k_default_file_video_max_change_interval_secs, "")
wall_conf_key(lock, screenshot_enabled, k_default_file_screenshot_enabled, "")
wall_conf_key(lock, screenshot_cache_enabled, k_default_file_screenshot_cache_enabled, "")
wall_conf_key(lock, screenshot_directory, k_default_file_screenshot_directory, "")
wall_conf_key(lock, screenshot_delay_ms, k_default_file_screenshot_delay_ms, "")
wall_conf_key(lock, screenshot_done_cmd, "", "")
wall_conf_key(lock, screenshot_reload_on_done, true, "Reload colors on success.")

// lock indicator settings
wall_conf_key(lock_indicator, enabled, true, "Enables indicator ring on the lock screen.")
wall_conf_key(lock_indicator, monitor, "all", "Determines which monitors will display the lock indicator.")
wall_conf_key(lock_indicator, image_path, "", "Path to the image to display in the center of the lock indicator.")
wall_conf_key(lock_indicator, visible_on_idle, false, "Enables showing the indicator when idle.")
wall_conf_key(lock_indicator, idle_timeout_ms, 5000, "Idle timeout in milliseconds.")
wall_conf_key(lock_indicator, clock_enabled, true, "Enables the clock in the center of the lock indicator.")
wall_conf_key(lock_indicator, clock_format, "%I:%M %p", "Clock format, should confirm to strftime format (https://en.cppreference.com/w/cpp/chrono/c/strftime).")
wall_conf_key(lock_indicator, ring_enabled, true, "Enables the outer ring.")
wall_conf_key(lock_indicator, ring_radius, 75, "Radius from the center of the indicator ring.")
wall_conf_key(lock_indicator, ring_thickness, 10, "Thickness of the ring.")
wall_conf_key(lock_indicator, ring_fill_color_input, "{color1}", "Ring fill color during input.")
wall_conf_key(lock_indicator, ring_fill_color_cleared, "#E5A445FF", "Ring fill color after the password has been cleared.")
wall_conf_key(lock_indicator, ring_fill_color_caps_lock, k_default_lock_indicator_ring_fill_color_input, "Ring fill color when caps lock is enabled.")
wall_conf_key(lock_indicator, ring_fill_color_verifying, "{color9}", "Ring fill color during password authentication.")
wall_conf_key(lock_indicator, ring_fill_color_wrong, "#7D3300FF", "Ring fill color if authentication failed.")
wall_conf_key(lock_indicator, ring_inner_enabled, true, "Enables the inner ring.")
wall_conf_key(lock_indicator, ring_inner_fill_color_input, k_default_background_color, "Fill color of the inner ring during input.")
wall_conf_key(lock_indicator, ring_inner_fill_color_cleared, k_default_background_color, "")
wall_conf_key(lock_indicator, ring_inner_fill_color_caps_lock, k_default_background_color, "")
wall_conf_key(lock_indicator, ring_inner_fill_color_verifying, k_default_background_color, "")
wall_conf_key(lock_indicator, ring_inner_fill_color_wrong, k_default_background_color, "")
wall_conf_key(lock_indicator, ring_border_color_input, k_default_border_color, "")
wall_conf_key(lock_indicator, ring_border_color_cleared, k_default_lock_indicator_ring_border_color_input, "")
wall_conf_key(lock_indicator, ring_border_color_caps_lock, k_default_lock_indicator_ring_border_color_input, "")
wall_conf_key(lock_indicator, ring_border_color_verifying, k_default_lock_indicator_ring_border_color_input, "")
wall_conf_key(lock_indicator, ring_border_color_wrong, k_default_lock_indicator_ring_border_color_input, "")
wall_conf_key(lock_indicator, ring_border_width, k_default_border_width, "")
wall_conf_key(lock_indicator, ring_inner_border_width, k_default_border_width, "")
wall_conf_key(lock_indicator, ring_highlight_color_keypress, "{color2}", "")
wall_conf_key(lock_indicator, ring_highlight_color_backspace, "{color8}", "")
wall_conf_key(lock_indicator, ring_highlight_arc, (M_PI / 3.0), "")
wall_conf_key(lock_indicator, ring_highlight_arc_thickness, 10, "")
wall_conf_key(lock_indicator, ring_highlight_arc_border_thickness, (M_PI / 128.0), "")
wall_conf_key(lock_indicator, ring_highlight_border_color, k_default_border_color, "")
wall_conf_key(lock_indicator, font, k_default_font_name, "")
wall_conf_key(lock_indicator, font_size, 0.0, "Font size of the indicator message, default is 1/3 of the radius.")
wall_conf_key(lock_indicator, font_color_input, k_default_font_color, "")
wall_conf_key(lock_indicator, font_color_cleared, k_default_font_color, "")
wall_conf_key(lock_indicator, font_color_caps_lock, "{color8}", "")
wall_conf_key(lock_indicator, font_color_verifying, k_default_font_color, "")
wall_conf_key(lock_indicator, font_color_wrong, k_default_font_color, "")
wall_conf_key(lock_indicator, message_input, "", "")
wall_conf_key(lock_indicator, message_cleared, "", "")
wall_conf_key(lock_indicator, message_caps_lock, " Caps", "")
wall_conf_key(lock_indicator, message_verifying, "Verifying", "")
wall_conf_key(lock_indicator, message_wrong, "Wrong", "")

// analog clock settings
wall_conf_key(lock_indicator, analog_clock_enabled, false, "Enables analog clock in the center of the indicator.")
wall_conf_key(lock_indicator, analog_clock_hour_hand_length, 50.0, "")
wall_conf_key(lock_indicator, analog_clock_minute_hand_length, 55.0, "")
wall_conf_key(lock_indicator, analog_clock_second_hand_length, 60.0, "")
wall_conf_key(lock_indicator, analog_clock_hour_hand_thickness, 3.0, "")
wall_conf_key(lock_indicator, analog_clock_minute_hand_thickness, 2.0, "")
wall_conf_key(lock_indicator, analog_clock_second_hand_thickness, 1.0, "")
wall_conf_key(lock_indicator, analog_clock_center_radius, 5.0, "")
wall_conf_key(lock_indicator, analog_clock_hour_marker_radius, 60.0, "")
wall_conf_key(lock_indicator, analog_clock_hour_marker_length, 5.0, "")
wall_conf_key(lock_indicator, analog_clock_hour_marker_thickness, 1.0, "")
wall_conf_key(lock_indicator, analog_clock_second_marker_radius, 60.0, "")
wall_conf_key(lock_indicator, analog_clock_second_marker_length, 1.0, "")
wall_conf_key(lock_indicator, analog_clock_second_marker_thickness, 1.0, "")
wall_conf_key(lock_indicator, analog_clock_hour_hand_enabled, true, "")
wall_conf_key(lock_indicator, analog_clock_minute_hand_enabled, true, "")
wall_conf_key(lock_indicator, analog_clock_second_hand_enabled, true, "")
wall_conf_key(lock_indicator, analog_clock_center_enabled, true, "")
wall_conf_key(lock_indicator, analog_clock_hour_marker_enabled, true, "")
wall_conf_key(lock_indicator, analog_clock_second_marker_enabled, true, "")
wall_conf_key(lock_indicator, analog_clock_hand_color_input, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_hand_color_cleared, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_hand_color_caps_lock, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_hand_color_verifying, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_hand_color_wrong, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_center_color_input, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_center_color_cleared, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_center_color_caps_lock, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_center_color_verifying, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_center_color_wrong, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_marker_color_input, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_marker_color_cleared, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_marker_color_caps_lock, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_marker_color_verifying, k_default_font_color, "")
wall_conf_key(lock_indicator, analog_clock_marker_color_wrong, k_default_font_color, "")

// bar settings
wall_conf_key(lock_bar, enabled, true, "Enables bar on the lock screen.")
wall_conf_key(lock_bar, monitor, "primary", "Determines which monitors will display the bar.")
wall_conf_key(lock_bar, visible_on_idle, false, "Enables showing the bar when idle.")
wall_conf_key(lock_bar, idle_timeout_ms, 5000, "Idle timeout in milliseconds.")
wall_conf_key(lock_bar, modules, "keyboard, network, battery, clock", "available modules are battery, caps_lock, clock, keyboard, network.")
wall_conf_key(lock_bar, module_separator, "  ", "String separating the modules.")
wall_conf_key(lock_bar, module_draw_on_empty, false, "Enables drawing the modules even if the module message is empty.")
wall_conf_key(lock_bar, font, k_default_font_name, "")
wall_conf_key(lock_bar, font_size, k_default_font_size, "")
wall_conf_key(lock_bar, font_color, k_default_font_color, "")
wall_conf_key(lock_bar, background_color, k_default_background_color, "")
wall_conf_key(lock_bar, border_color, k_default_border_color, "")
wall_conf_key(lock_bar, border_width, k_default_border_width, "")
wall_conf_key(lock_bar, corner_radius, 0, "Radius of the corners, default is 1/2 the height.")
wall_conf_key(lock_bar, alignment, "bottom-center", "Alignment of the bar, options are top-left, top-center, top-right, bottom-left, bottom-center, bottom-right.")
wall_conf_key(lock_bar, left_padding, 10, "Padding on the left side of the bar.")
wall_conf_key(lock_bar, right_padding, 10, "Padding on the right side of the bar.")
wall_conf_key(lock_bar, top_padding, 10, "Padding on the top side of the bar.")
wall_conf_key(lock_bar, bottom_padding, 10, "Padding on the bottom side of the bar.")
wall_conf_key(lock_bar, text_top_bottom_margin, 2, "Margin between the top and bottom of the text.")

wall_conf_key(lock_bar, keyboard_format, "{caps_lock}{layout} 󰌓", "Format of the keyboard module.")
wall_conf_key(lock_bar, layout_guess_short_name, true, "Parses out the name between the parens in the keyboard layout (e.g. English (US) -> US)")

wall_conf_key(lock_bar, caps_locked_format, " ", "Format of the caps_lock module when caps lock is enabled.")
wall_conf_key(lock_bar, caps_unlocked_format, "", "Format of the caps_lock module when caps lock is disabled.")

wall_conf_key(lock_bar, network_format_wifi, " ", "Format of the network module when a wifi connection is detected. Available replacements {ipaddr} {ifname}.")
wall_conf_key(lock_bar, network_format_ethernet, "󰈀", "Format of the network module when no wifi connection is detected and an ethernet connection is connected. Available replacements {ipaddr} {ifname}.")
wall_conf_key(lock_bar, network_format_disconnected, "󰖪", "Format of the network module when no connection is detected.")
wall_conf_key(lock_bar, network_format_ifname_max_length, 50, "Maximum length of the interface name.")
wall_conf_key(lock_bar, network_update_interval_secs, 60, "Update interval in seconds for the network module.")

wall_conf_key(lock_bar, battery_level_icons, " , , , , ", "Icons for the battery level, separated by commas.")
wall_conf_key(lock_bar, battery_plugged_format, "{icon}   {capacity}%", "Format of the battery module when the battery is charging.")
wall_conf_key(lock_bar, battery_charging_format, "{icon}   {capacity}%", "Format of the battery module when the battery is charging.")
wall_conf_key(lock_bar, battery_discharging_format, "{icon}  {capacity}%", "Format of the battery module when the battery is discharging.")
wall_conf_key(lock_bar, battery_font_color, k_default_font_color, "")
wall_conf_key(lock_bar, battery_not_found, "", "Message to display when the battery is not found.")
wall_conf_key(lock_bar, battery_update_interval_secs, 60, "Update interval in seconds for the battery module.")

wall_conf_key(lock_bar, clock_format, "%a, %b %d", "Default is \"Day of week, Month Year.\". Should confirm to strftime (https://en.cppreference.com/w/cpp/chrono/c/strftime) format.")

// log settings
wall_conf_key(log, threads, 1, "Number of threads the logger should use.")
wall_conf_key(log, level, "info", "Default log level.")
wall_conf_key(log, line_count, 1 << 18, "Maximum log line count.")
wall_conf_key(log, file_size, 1 << 25, "Maximum log file size.")
wall_conf_key(log, file, "wallock.log", "Log file name.")


[[maybe_unused]] auto setup_settings() -> void;

[[maybe_unused]] auto print_default_config(std::ostream& stream) -> void;

[[maybe_unused]] auto print_default_config_markdown(std::ostream& stream) -> void;

// clang-format on
}  // namespace wall::conf
