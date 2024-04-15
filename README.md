



# Wallock

[![Actions Status](https://github.com/dpayne/wallock/workflows/test/badge.svg)](https://github.com/dpayne/wallock/actions)
[![Actions Status](https://github.com/dpayne/wallock/workflows/tidy/badge.svg)](https://github.com/dpayne/wallock/actions)
[![Actions Status](https://github.com/dpayne/wallock/workflows/format/badge.svg)](https://github.com/dpayne/wallock/actions)
[![codecov](https://codecov.io/gh/dpayne/wallock/graph/badge.svg?token=WKMH3L9ZGV)](https://codecov.io/gh/dpayne/wallock)

wallock is a wallpaper and lock screen that enables macos like lock screens and wallapers on wayland. It is designed to work with wlroots based wayland compositors (sway, hyprland, etc..).

## Demo

[![Example 1](https://img.youtube.com/vi/an07XFvCkIc/0.jpg)](https://www.youtube.com/watch?v=an07XFvCkIc)

![Example 2](https://github.com/dpayne/wallock/assets/1331138/7a9e26c3-a4b9-4497-8b94-164c70466d6d)

# Installation

## Arch Linux

```
yay -S wallock-git
```

# Build from source

## Dependencies

### Arch Linux

```
sudo pacman -S base-devel git cmake wayland wayland-protocols egl-wayland libxkbcommon mesa mpv cairo ttf-firacode-nerd otf-firamono-nerd libdrm
```

**Aur package:** https://aur.archlinux.org/packages/wallock-git

### Ubuntu

```
sudo apt-get install build-essential git cmake libwayland-dev libwayland-egl-backend-dev wayland-protocols libxkbcommon-dev libegl1-mesa-dev libcairo2-dev libmpv-dev libpam0g-dev libudev-dev libdrm-dev
```

Installing the fira code font is optional, but the default config uses the font. To install the font on Ubuntu, run the following commands:

```
sudo apt install wget fontconfig
wget -P ~/.local/share/fonts https://github.com/ryanoasis/nerd-fonts/releases/download/v3.1.1/FiraCode.zip
cd ~/.local/share/fonts && unzip FiraCode.zip && rm FiraCode.zip && fc-cache -fv
```

## Building

Use the following command to build and run the executable target.

```
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

# Installation is needed to install the lock screen pam module. Otherwise the lock screen will never authenticate.
sudo cmake --install build
```

# Usage

Run `wallock` to start the program. By default it will start in wallpaper mode. Then run `wallock -o lock` to lock the screen.

While `walllock` is running, commands can be sent via `wallock -o <command>`. The following commands are supported: lock, stop, next, full_reload, reload.

* `lock` will lock the screen.
* `stop` will stop the program and exit.
* `next` will load the next video in the list.
* `reload` will reload the color scheme and some other configuration options but does not trigger a full reload. This is useful when using thing like pywal.
* `full_reload` will reload the configuration file and triggers a full reload. This is useful when changing the file paths in the configuration file. This will do nothing when lock screen is active.

## Lock screen without wallpaper

To start the lock screen without the running the wallpaper run `wallock -s`, and disable the wallpaper in the configuration file.
```
wallpaper_enabled=false
```

# Signals

`wallock` will listen for the following signals:

* `SIGUSR1` - Triggers the `reload` command.
* `SIGUSR2` - Triggers the `full_reload` command. This is *not* available when lock screen is active.

# Configuration

The default configuration file is read from `~/.config/wallock/config`. The default color scheme file is read from `~/.config/wallock/color_scheme`. A full configuration list is available at [Configuration.md](./Configuration.md).

## Example Configurations

#### Basic

```
# path for the source of the videos, the default path is ~/.wallpapers
file_path=~/.wallpapers
```

#### With Color Scheme

```
file_path=~/.wallpapers
color_scheme_file=~/.config/wallock/color_scheme
```

By the default, the it looks for a color scheme file at `~/.config/wallock/color_scheme`. Each color in the color scheme is defined as a hex value with an optional alpha channel (RRGGBBAA).

```
color_background=#000000C0
color_foreground=#FFFFFFFF
color_0=#000000FF
...
color_15=#FFFFFFFF
```

#### Multiple Monitors

Multiple monitors are supported by default. The primary monitor is set in the configuration file.

```
# Sets the primary monitor to HDMI-A-1
monitor_primary=HDMI-A-1

# Only shows the lock indicator on the primary monitor
lock_indicator_monitor=primary

# keeps the videos in the same order across multiple monitors
file_keep_same_order=true
```

#### More complex example

```
# Set two different paths for the lock screen and the wallpaper
lock_path=~/.wallpapers/images
wallpaper_path=~/.wallpapers/videos

# Pause the video 10 seconds after unlocking the screen
wallpaper_pause_after_unlock=true
wallpaper_pause_after_unlock_delay_secs=10

# set the lock indicator and bar to the primary monitor
lock_bar_monitor=primary
lock_indicator_monitor=primary
monitor_primary=HDMI-A-1

# Keeps the same order across multiple monitors
file_keep_same_order=true

# Enable screenshots of the video source and run wal to generate a color scheme
file_screenshot_enabled=true
file_screenshot_done_cmd=wal -i "{filename}"

# Enables the analog clock lock indicator
lock_indicator_analog_clock_enabled=true
lock_indicator_analog_clock_second_hand_enabled=false
lock_indicator_analog_clock_hour_marker_enabled=true
lock_indicator_analog_clock_second_marker_enabled=false


# Sets which modules are shown in the lock bar and in what order
lock_bar_modules=keyboard, network, battery, clock

```

# Using with pywal

The lock bar and lock indicator can be themed with pywal. To this, add a template for wallock to the pywal templates directory.

An example template is below:

```
color_background={background}
color_foreground={foreground}
color_0={color0}
color_1={color1}
color_2={color2}
color_3={color3}
color_4={color4}
color_5={color5}
color_6={color6}
color_7={color7}
color_8={color8}
color_9={color9}
color_10={color10}
color_11={color11}
color_12={color12}
color_13={color13}
color_14={color14}
color_15={color15}
```

Assuming the template is saved as `~/.config/wal/wallock_color_scheme`, then add the following to the wallock config file:

```
color_scheme_file=~/.cache/wallock_color_scheme
```

Then run the following command to generate the color scheme.

```
wal -i /path/to/image.jpg
```

If wallock is already running then you can run the `reload` command to reload the color scheme.

```
wallock -o reload
```

### Screenshots with pywal

It is possible to use the current video as the theme for pywal. To do this, the follow the pywal instructions above to setup the color scheme. Then add the following to the wallock config file:

```
file_screenshot_enabled=true
file_screenshot_done_cmd=wal -q -i "{filename}"
```
This will take a screenshot of the video source each time a new video is loaded. It will then run pywal to generate the color scheme. The wallock's color scheme is automatically reloaded after running the screenshot command.

Note the screenshot here will be a screenshot of the video source, it will *not* be a screenshot of the desktop.

By default, screenshots are taken 1 second into the video is cached in the `~/.cache/wallock` directory. If the screenshot exists in cache, then the cache version is used immediately instead of taking a screenshot.

## Example Video Sources

For arch users, there are existing packages with aerial videos packages on AUR.

* [aerial-2k-videos](https://aur.archlinux.org/packages/aerial-2k-videos)
* [aerial-4k-videos](https://aur.archlinux.org/packages/aerial-4k-videos)

# Troubleshooting

If you have issues with crashes or the video/image popping up in a new window instead of in the background, then try disabling hardware rendering.

```
general_force_software_rendering=true
```

### Nvidia

Nvidia support should be considered pretty unstable. It seems to mostly work for me with the latest drivers and latest source build of nvidia-egl-wayland.

# Contributing

When submitting a pull request, please run the formatter and test suite locally to ensure that the changes pass all checks. More info is at [Contributing.md](./Contributing.md).

# Security Considerations

This is a new project that is using some non-trivial libraries (libmpv, cairo, etc..). If you are concerned about security, I would recommend not using this project until it has been more thoroughly vetted.

# Acknowledgements

This project started out as a way to duplicate [xscreensaver-aerial](https://github.com/graysky2/xscreensaver-aerial) on wayland.

A lot of code was taken and modified from both [swaylock](https://github.com/swaywm/swaylock) and [swaylock-effects](https://github.com/mortie/swaylock-effects) . I would strongly recommend either project if you do not need the features `wallock` provides.

[mpvpaper](https://github.com/GhostNaN/mpvpaper) was also used as a reference for the mpv integration.
