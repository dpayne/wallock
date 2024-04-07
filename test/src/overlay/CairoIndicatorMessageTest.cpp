#include <cairo.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestUtils.hpp"
#include "conf/ConfigDefaultSettings.hpp"
#include "overlay/CairoFontCache.hpp"
#include "overlay/CairoIndicatorMessage.hpp"

TEST(CairoIndicatorMessageTest, test_cairo_indicator_message_clock_disabled) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_clock_enabled, false);

    wall::CairoFontCache font_cache{config, CAIRO_SUBPIXEL_ORDER_DEFAULT};
    font_cache.load_font("FiraCode Nerd Font", 12);

    const auto time = wall::TestUtils::convert_date_string_to_time_point("2001-01-01 12:00:00");

    wall::CairoIndicatorMessage indicator_message{config};
    indicator_message.update_message(wall::State::Input, time);
    EXPECT_EQ(indicator_message.get_message(), "");
    auto text_width = indicator_message.get_text_width(font_cache);
    EXPECT_EQ(text_width, 0.0);
}

TEST(CairoIndicatorMessageTest, test_cairo_indicator_message_clock_enabled) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_clock_enabled, true);

    wall::CairoFontCache font_cache{config, CAIRO_SUBPIXEL_ORDER_DEFAULT};
    font_cache.load_font("FiraCode Nerd Font", 12);

    const auto time = wall::TestUtils::convert_date_string_to_time_point("2001-01-01 12:00:00");

    wall::CairoIndicatorMessage indicator_message{config};
    indicator_message.update_message(wall::State::Input, time);
    EXPECT_EQ(indicator_message.get_message(), "12:00 PM");
    auto text_width = indicator_message.get_text_width(font_cache);
    EXPECT_EQ(text_width, 55.999999999999993);
}

TEST(CairoIndicatorMessageTest, test_cairo_indicator_caps_lock_message) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_clock_enabled, true);

    wall::CairoFontCache font_cache{config, CAIRO_SUBPIXEL_ORDER_DEFAULT};
    font_cache.load_font("FiraCode Nerd Font", 12);

    const auto time = wall::TestUtils::convert_date_string_to_time_point("2001-01-01 12:00:00");

    wall::CairoIndicatorMessage indicator_message{config};
    indicator_message.update_message(wall::State::CapsLock, time);
    EXPECT_EQ(indicator_message.get_message(), " Caps");
    auto text_width = indicator_message.get_text_width(font_cache);
    EXPECT_EQ(text_width, 41.999999999999993);
}

TEST(CairoIndicatorMessageTest, test_cairo_indicator_message_draw) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_clock_enabled, true);

    wall::CairoFontCache font_cache{config, CAIRO_SUBPIXEL_ORDER_DEFAULT};
    font_cache.load_font("FiraCode Nerd Font", 12);

    const auto time = wall::TestUtils::convert_date_string_to_time_point("2001-01-01 12:00:00");

    const auto width = 100.0;
    const auto height = 100.0;

    auto* buffer = (unsigned char*)malloc(4 * width * height);
    auto* surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, width, height, 4 * width);
    auto* cairo = cairo_create(surface);

    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    cairo_set_source_rgb(cairo, 0, 0, 0);
    cairo_paint(cairo);

    wall::CairoIndicatorMessage indicator_message{config};
    indicator_message.update_message(wall::State::Keypress, time);
    indicator_message.draw(cairo, wall::State::Keypress, font_cache, width / 2.0, height / 2.0);

    cairo_surface_write_to_png(surface, "/tmp/test_indicator_message_clock.png");

    config.set(wall::conf::k_lock_indicator_clock_enabled, false);
    indicator_message.update_settings();

    cairo_set_source_rgb(cairo, 0, 0, 0);
    cairo_paint(cairo);

    indicator_message.update_message(wall::State::CapsLock, time);
    indicator_message.draw(cairo, wall::State::CapsLock, font_cache, width / 2.0, height / 2.0);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_message_caps_lock.png");

    cairo_surface_flush(surface);
    cairo_surface_finish(surface);

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);
    free(buffer);
}
