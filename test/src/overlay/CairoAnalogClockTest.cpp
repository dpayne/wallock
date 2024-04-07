#include <cairo.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "conf/ConfigDefaultSettings.hpp"
#include "overlay/CairoAnalogClockElement.hpp"

TEST(CairoAnalogClockTest, test_cairo_analog_clock) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");

    const auto second_hand_length = wall_conf_get(config, lock_indicator, analog_clock_second_hand_length);
    const auto height = second_hand_length * 2.0 + 10.0;
    const auto width = height;

    auto* buffer = (unsigned char*)malloc(4 * width * height);
    auto* surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, width, height, 4 * width);
    auto* cairo = cairo_create(surface);

    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    const auto center_x = width / 2.0;
    const auto center_y = height / 2.0;

    wall::CairoAnalogClockElement clock{config};

    // paint black background
    cairo_set_source_rgb(cairo, 0, 0, 0);
    cairo_paint(cairo);

    clock.draw(cairo, center_x, center_y, wall::State::Keypress);

    cairo_surface_write_to_png(surface, "/tmp/test_analog_clock.png");

    cairo_surface_flush(surface);
    cairo_surface_finish(surface);

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);

    free(buffer);
}

TEST(CairoAnalogClockTest, test_update_interval) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");

    wall::CairoAnalogClockElement clock{config};
    ASSERT_FALSE(clock.should_redraw(std::chrono::milliseconds(100)));
    ASSERT_TRUE(clock.should_redraw(std::chrono::milliseconds(1001)));

    config.set(wall::conf::k_lock_indicator_analog_clock_second_hand_enabled, false);
    clock.update_settings();
    ASSERT_FALSE(clock.should_redraw(std::chrono::milliseconds(2000)));
    ASSERT_TRUE(clock.should_redraw(std::chrono::milliseconds(60001)));

    config.set(wall::conf::k_lock_indicator_analog_clock_minute_hand_enabled, false);
    clock.update_settings();
    ASSERT_FALSE(clock.should_redraw(std::chrono::milliseconds(60002)));
    ASSERT_TRUE(clock.should_redraw(std::chrono::milliseconds(3600001)));

    config.set(wall::conf::k_lock_indicator_analog_clock_hour_hand_enabled, false);
    clock.update_settings();
    ASSERT_FALSE(clock.should_redraw(std::chrono::milliseconds(3600002)));
    ASSERT_FALSE(clock.should_redraw(std::chrono::milliseconds(36000000)));
}
