# Configuration Options

The default configuration file location will be`~/.config/wallock/config`.

## Color Scheme

The default color scheme location will be `~/.config/wallock/color_scheme`. The default color scheme is as follows

| Color | Default |
|  --------  |  -------  |
| color_background | #000000 | Default color scheme. |
| color_foreground | #FFFFFF |  |
| color_0 | #000000 |  |
| color_1 | #337D00 |  |
| color_2 | #33DB00 |  |
| color_3 | #DB3300 |  |
| color_4 | #000000 |  |
| color_5 | #000000 |  |
| color_6 | #000000 |  |
| color_7 | #000000 |  |
| color_8 | #E5A445 |  |
| color_9 | #3300FF |  |
| color_10 | #FFFFFF |  |
| color_11 | #000000 |  |
| color_12 | #000000 |  |
| color_13 | #000000 |  |
| color_14 | #000000 |  |
| color_15 | #000000 |  |


### General Options
| Key | Default | Description |
|  --------  |  -------  | -------  |
| config_file | config | Configuration file path. |
| color_scheme_file | color_scheme | Configuration file path. |
| debug_mode | false | Enables debugging options. |
| command_ignore_is_running | false | Ignores if another instance is running. |
| command_name |  |  |
| command_socket_backlog | 128 | Number of connections to allow in the socket backlog. |
| command_socket_filename | wallock.sock | Socket filename. |
| start_lock | false | Starts lock immediately on startup. |
| password_allow_empty | false | Allow empty passwords. |
| password_max_length | 1024 | Maximum password length. |
| font_name | FiraCode Nerd Font | Default font name. |
| font_size | 20.000000 | Default font size. |
| font_color | {color10}FF | Default font color. |
| background_color | {background}C0 | Default background color. |
| border_color | {color6}FF | Default border color. |
| border_width | 2 | Default border width. |
| monitor_primary |  | Set primary monitor |
| grace_period_secs | 2 | Grace period in seconds for disabling password authentication. |
| general_daemonize | true | Daemonize, run in background. |
| general_force_software_rendering | false | Forces software rendering. |
| general_mpv_log_enabled | false | Enables mpv logging. |


### Wallpaper and Lock Screen Options

These options have an equivalent setting that is specific to the wallpaper or lock screen. The more specific option takes precedence if set. For example, if `file_path` is set and both `lock_path` and `wallpaper_path` are not set, then `file_path` will be used for both the lock screen and the wallpaper. This convention follows for all options that have the `file_` prefix.

| Key | Default | Description |
|  --------  |  -------  | -------  |
| file_path | ~/.wallpapers/ | Path for wallpaper and lock screen resources. |
| file_extensions | jpg,jpeg,png,mp4,mov,webm,avi,wmv,flv,mkv | Allowed extensions for wallpaper and lock screen resources. |
| file_mute | true | Enabling or disabling muting of resources. |
| file_fit | fill | Options are fit or fill. |
| file_sort_order | random | Sort order for resources, options are random or alpha. |
| file_loop | false | Loop the first loaded resource. |
| file_keep_same_order | false | Enforces the same ordering of resources between multiple monitors. |
| file_image_change_interval_secs | 900 | How long to display an image for before rotating. |
| file_video_preload_secs | 1 | How long before the current resource finishes before loading the next. Note this may cutoff the end of a video. |
| file_video_max_change_interval_secs | 0 | Maximum time in seconds a video is allowed to play before rotating, default is unlimited. |
| file_screenshot_enabled | false | Enables taking a screenshot after a new resource has been loaded. |
| file_screenshot_cache_enabled | true | Caches the screenshot of a video. |
| file_screenshot_directory |  | Screenshot location, default will be ~/.cache/wallock. |
| file_screenshot_delay_ms | 1000 | How long to wait until after a resource loads to take a screenshot. |
| file_screenshot_done_cmd |  | Command to run after a screenshot has been taken. |
| file_screenshot_format | jpg | Screenshot format, note png can be very slow and large, jpg is recommended. |
| file_screenshot_filename | {filename}.{format} | Screenshot filename. |
| file_screenshot_reload_on_done | true | Reload colors on success. |

### Wallpaper Options

| Key | Default | Description |
|  --------  |  -------  | -------  |
| wallpaper_enabled | true | Enables wallpaper. |
| wallpaper_pause_after_unlock | false | Pauses the wallpaper after unlocking. |
| wallpaper_dismiss_after_pause | false | Dismisses the wallpaper after pausing. |
| wallpaper_pause_after_unlock_delay_secs | 10 | Delay in seconds before pausing the wallpaper after unlocking. |
| wallpaper_path |  | These are optional and override the file settings for the wallpaper. |
| wallpaper_extensions | jpg,jpeg,png,mp4,mov,webm,avi,wmv,flv,mkv |  |
| wallpaper_mute | true |  |
| wallpaper_fit | fill |  |
| wallpaper_sort_order | random |  |
| wallpaper_loop | false |  |
| wallpaper_keep_same_order | false |  |
| wallpaper_image_change_interval_secs | 900 |  |
| wallpaper_video_preload_secs | 1 |  |
| wallpaper_video_max_change_interval_secs | 0 |  |
| wallpaper_screenshot_enabled | false |  |
| wallpaper_screenshot_cache_enabled | true |  |
| wallpaper_screenshot_directory |  |  |
| wallpaper_screenshot_delay_ms | 1000 |  |
| wallpaper_screenshot_done_cmd |  |  |
| wallpaper_bar_enabled | false |  |
| wallpaper_indicator_enabled | false |  |
| wallpaper_screenshot_reload_on_done | true | Reload colors on success. |

### Lock Screen Options

| Key | Default | Description |
|  --------  |  -------  | -------  |
| lock_path |  | These are optional and override the file settings for the lock screen. |
| lock_extensions | jpg,jpeg,png,mp4,mov,webm,avi,wmv,flv,mkv |  |
| lock_mute | true |  |
| lock_fit | fill |  |
| lock_sort_order | random |  |
| lock_loop | false |  |
| lock_keep_same_order | false |  |
| lock_image_change_interval_secs | 900 |  |
| lock_video_preload_secs | 1 |  |
| lock_video_max_change_interval_secs | 0 |  |
| lock_screenshot_enabled | false |  |
| lock_screenshot_cache_enabled | true |  |
| lock_screenshot_directory |  |  |
| lock_screenshot_delay_ms | 1000 |  |
| lock_screenshot_done_cmd |  |  |
| lock_screenshot_reload_on_done | true | Reload colors on success. |

#### Lock Indicator Options
| Key | Default | Description |
|  --------  |  -------  | -------  |
| lock_indicator_enabled | true | Enables indicator ring on the lock screen. |
| lock_indicator_monitor | all | Determines which monitors will display the lock indicator. |
| lock_indicator_image_path |  | Path to the image to display in the center of the lock indicator. |
| lock_indicator_visible_on_idle | false | Enables showing the indicator when idle. |
| lock_indicator_idle_timeout_ms | 5000 | Idle timeout in milliseconds. |
| lock_indicator_clock_enabled | true | Enables the clock in the center of the lock indicator. |
| lock_indicator_clock_format | %I:%M %p | Clock format, should confirm to strftime format (https://en.cppreference.com/w/cpp/chrono/c/strftime). |
| lock_indicator_ring_enabled | true | Enables the outer ring. |
| lock_indicator_ring_radius | 75 | Radius from the center of the indicator ring. |
| lock_indicator_ring_thickness | 10 | Thickness of the ring. |
| lock_indicator_ring_fill_color_input | {color1} | Ring fill color during input. |
| lock_indicator_ring_fill_color_cleared | #E5A445FF | Ring fill color after the password has been cleared. |
| lock_indicator_ring_fill_color_caps_lock | {color1} | Ring fill color when caps lock is enabled. |
| lock_indicator_ring_fill_color_verifying | {color9} | Ring fill color during password authentication. |
| lock_indicator_ring_fill_color_wrong | #7D3300FF | Ring fill color if authentication failed. |
| lock_indicator_ring_inner_enabled | true | Enables the inner ring. |
| lock_indicator_ring_inner_fill_color_input | {background}C0 | Fill color of the inner ring during input. |
| lock_indicator_ring_inner_fill_color_cleared | {background}C0 |  |
| lock_indicator_ring_inner_fill_color_caps_lock | {background}C0 |  |
| lock_indicator_ring_inner_fill_color_verifying | {background}C0 |  |
| lock_indicator_ring_inner_fill_color_wrong | {background}C0 |  |
| lock_indicator_ring_border_color_input | {color6}FF |  |
| lock_indicator_ring_border_color_cleared | {color6}FF |  |
| lock_indicator_ring_border_color_caps_lock | {color6}FF |  |
| lock_indicator_ring_border_color_verifying | {color6}FF |  |
| lock_indicator_ring_border_color_wrong | {color6}FF |  |
| lock_indicator_ring_border_width | 2 |  |
| lock_indicator_ring_inner_border_width | 2 |  |
| lock_indicator_ring_highlight_color_keypress | {color2} |  |
| lock_indicator_ring_highlight_color_backspace | {color8} |  |
| lock_indicator_ring_highlight_arc | 1.047198 |  |
| lock_indicator_ring_highlight_arc_thickness | 10 |  |
| lock_indicator_ring_highlight_arc_border_thickness | 0.024544 |  |
| lock_indicator_ring_highlight_border_color | {color6}FF |  |
| lock_indicator_font | FiraCode Nerd Font |  |
| lock_indicator_font_size | 0.000000 | Font size of the indicator message, default is 1/3 of the radius. |
| lock_indicator_font_color_input | {color10}FF |  |
| lock_indicator_font_color_cleared | {color10}FF |  |
| lock_indicator_font_color_caps_lock | {color8} |  |
| lock_indicator_font_color_verifying | {color10}FF |  |
| lock_indicator_font_color_wrong | {color10}FF |  |
| lock_indicator_message_input |  |  |
| lock_indicator_message_cleared |  |  |
| lock_indicator_message_caps_lock |  Caps |  |
| lock_indicator_message_verifying | Verifying |  |
| lock_indicator_message_wrong | Wrong |  |
| lock_indicator_analog_clock_enabled | false | Enables analog clock in the center of the indicator. |
| lock_indicator_analog_clock_hour_hand_length | 50.000000 |  |
| lock_indicator_analog_clock_minute_hand_length | 55.000000 |  |
| lock_indicator_analog_clock_second_hand_length | 60.000000 |  |
| lock_indicator_analog_clock_hour_hand_thickness | 3.000000 |  |
| lock_indicator_analog_clock_minute_hand_thickness | 2.000000 |  |
| lock_indicator_analog_clock_second_hand_thickness | 1.000000 |  |
| lock_indicator_analog_clock_center_radius | 5.000000 |  |
| lock_indicator_analog_clock_hour_marker_radius | 60.000000 |  |
| lock_indicator_analog_clock_hour_marker_length | 5.000000 |  |
| lock_indicator_analog_clock_hour_marker_thickness | 1.000000 |  |
| lock_indicator_analog_clock_second_marker_radius | 60.000000 |  |
| lock_indicator_analog_clock_second_marker_length | 1.000000 |  |
| lock_indicator_analog_clock_second_marker_thickness | 1.000000 |  |
| lock_indicator_analog_clock_hour_hand_enabled | true |  |
| lock_indicator_analog_clock_minute_hand_enabled | true |  |
| lock_indicator_analog_clock_second_hand_enabled | true |  |
| lock_indicator_analog_clock_center_enabled | true |  |
| lock_indicator_analog_clock_hour_marker_enabled | true |  |
| lock_indicator_analog_clock_second_marker_enabled | true |  |
| lock_indicator_analog_clock_hand_color_input | {color10}FF |  |
| lock_indicator_analog_clock_hand_color_cleared | {color10}FF |  |
| lock_indicator_analog_clock_hand_color_caps_lock | {color10}FF |  |
| lock_indicator_analog_clock_hand_color_verifying | {color10}FF |  |
| lock_indicator_analog_clock_hand_color_wrong | {color10}FF |  |
| lock_indicator_analog_clock_center_color_input | {color10}FF |  |
| lock_indicator_analog_clock_center_color_cleared | {color10}FF |  |
| lock_indicator_analog_clock_center_color_caps_lock | {color10}FF |  |
| lock_indicator_analog_clock_center_color_verifying | {color10}FF |  |
| lock_indicator_analog_clock_center_color_wrong | {color10}FF |  |
| lock_indicator_analog_clock_marker_color_input | {color10}FF |  |
| lock_indicator_analog_clock_marker_color_cleared | {color10}FF |  |
| lock_indicator_analog_clock_marker_color_caps_lock | {color10}FF |  |
| lock_indicator_analog_clock_marker_color_verifying | {color10}FF |  |
| lock_indicator_analog_clock_marker_color_wrong | {color10}FF |  |

### Lock Bar Options

These are options for the bar that is shown on the lock screen.

| Key | Default | Description |
|  --------  |  -------  | -------  |
| lock_bar_enabled | true | Enables bar on the lock screen. |
| lock_bar_monitor | primary | Determines which monitors will display the bar. |
| lock_bar_visible_on_idle | false | Enables showing the bar when idle. |
| lock_bar_idle_timeout_ms | 5000 | Idle timeout in milliseconds. |
| lock_bar_modules | keyboard, network, battery, clock | available modules are battery, caps_lock, clock, keyboard, network. |
| lock_bar_module_separator |    | String separating the modules. |
| lock_bar_module_draw_on_empty | false | Enables drawing the modules even if the module message is empty. |
| lock_bar_font | FiraCode Nerd Font |  |
| lock_bar_font_size | 20.000000 |  |
| lock_bar_font_color | {color10}FF |  |
| lock_bar_background_color | {background}C0 |  |
| lock_bar_border_color | {color6}FF |  |
| lock_bar_border_width | 2 |  |
| lock_bar_corner_radius | 0 | Radius of the corners, default is 1/2 the height. |
| lock_bar_alignment | bottom-center | Alignment of the bar, options are top-left, top-center, top-right, bottom-left, bottom-center, bottom-right. |
| lock_bar_left_padding | 10 | Padding on the left side of the bar. |
| lock_bar_right_padding | 10 | Padding on the right side of the bar. |
| lock_bar_top_padding | 10 | Padding on the top side of the bar. |
| lock_bar_bottom_padding | 10 | Padding on the bottom side of the bar. |
| lock_bar_text_top_bottom_margin | 2 | Margin between the top and bottom of the text. |
| lock_bar_keyboard_format | {caps_lock}{layout} 󰌓 | Format of the keyboard module. |
| lock_bar_layout_guess_short_name | true | Parses out the name between the parens in the keyboard layout (e.g. English (US) -> US) |
| lock_bar_network_format_wifi |   | Format of the network module when a wifi connection is detected. Available replacements {ipaddr} {ifname}. |
| lock_bar_network_format_ethernet | 󰈀 | Format of the network module when no wifi connection is detected and an ethernet connection is connected. Available replacements {ipaddr} {ifname}. |
| lock_bar_network_format_disconnected | 󰖪 | Format of the network module when no connection is detected. |
| lock_bar_network_format_ifname_max_length | 50 | Maximum length of the interface name. |
| lock_bar_network_update_interval_secs | 60 | Update interval in seconds for the network module. |
| lock_bar_caps_locked_format |   | Format of the caps_lock module when caps lock is enabled. |
| lock_bar_caps_unlocked_format |  | Format of the caps_lock module when caps lock is disabled. |
| lock_bar_battery_level_icons |  , , , ,  | Icons for the battery level, separated by commas. |
| lock_bar_battery_not_found |  | Message to display when the battery is not found. |
| lock_bar_battery_plugged_format | {icon}   {capacity}% | Format of the battery module when the battery is charging. |
| lock_bar_battery_charging_format | {icon}   {capacity}% | Format of the battery module when the battery is charging. |
| lock_bar_battery_discharging_format | {icon}  {capacity}% | Format of the battery module when the battery is discharging. |
| lock_bar_battery_font_color | {color10}FF |  |
| lock_bar_battery_update_interval_secs | 60 | Update interval in seconds for the battery module. |
| lock_bar_clock_format | %a, %b %d | Default is "Day of week, Month Year.". Should confirm to strftime (https://en.cppreference.com/w/cpp/chrono/c/strftime) format. |

### Log Options
| Key | Default | Description |
|  --------  |  -------  | -------  |
| log_level | info | Default log level. |
| log_threads | 1 | Number of threads the logger should use. |
| log_line_count | 262144 | Maximum log line count. |
| log_file_size | 33554432 | Maximum log file size. |
| log_file | wallock.log | Log file name. |
