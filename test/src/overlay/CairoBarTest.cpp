#include <cairo.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "conf/ConfigDefaultSettings.hpp"
#include "overlay/CairoBarElement.hpp"
#include "overlay/CairoFontCache.hpp"

TEST(CairoBarTest, test_bar_alignment) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_alignment, "top-left");
    config.set(wall::conf::k_lock_bar_left_padding, 10);
    config.set(wall::conf::k_lock_bar_right_padding, 10);
    config.set(wall::conf::k_lock_bar_top_padding, 10);
    config.set(wall::conf::k_lock_bar_bottom_padding, 10);

    wall::CairoFontCache font_cache{config, CAIRO_SUBPIXEL_ORDER_DEFAULT};
    font_cache.load_font("FiraCode Nerd Font", 12);

    std::string message = "hello world!";

    wall::CairoBarElement bar{config};
    auto [width, height] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    EXPECT_EQ(width, 102);
    EXPECT_EQ(height, 18);

    auto [x_pos, y_pos] = bar.get_position_size(1920, 1080, width, height);

    EXPECT_EQ(x_pos, 10);
    EXPECT_EQ(y_pos, 10);

    config.set(wall::conf::k_lock_bar_alignment, "top-center");
    config.set(wall::conf::k_lock_bar_left_padding, 10);
    config.set(wall::conf::k_lock_bar_right_padding, 10);
    config.set(wall::conf::k_lock_bar_top_padding, 20);
    config.set(wall::conf::k_lock_bar_bottom_padding, 10);
    bar.update_settings();

    auto [width1, height1] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    EXPECT_EQ(width1, 102);
    EXPECT_EQ(height1, 18);

    auto [x_pos1, y_pos1] = bar.get_position_size(1920, 1080, width, height);

    EXPECT_EQ(x_pos1, 909);
    EXPECT_EQ(y_pos1, 20);

    config.set(wall::conf::k_lock_bar_alignment, "top-right");
    config.set(wall::conf::k_lock_bar_left_padding, 10);
    config.set(wall::conf::k_lock_bar_right_padding, 20);
    config.set(wall::conf::k_lock_bar_top_padding, 10);
    config.set(wall::conf::k_lock_bar_bottom_padding, 10);
    bar.update_settings();

    auto [width2, height2] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    EXPECT_EQ(width2, 102);
    EXPECT_EQ(height2, 18);

    auto [x_pos2, y_pos2] = bar.get_position_size(1920, 1080, width, height);

    EXPECT_EQ(x_pos2, 1920 - 102 - 20);
    EXPECT_EQ(y_pos2, 10);

    config.set(wall::conf::k_lock_bar_alignment, "bottom-right");
    config.set(wall::conf::k_lock_bar_left_padding, 10);
    config.set(wall::conf::k_lock_bar_right_padding, 10);
    config.set(wall::conf::k_lock_bar_top_padding, 10);
    config.set(wall::conf::k_lock_bar_bottom_padding, 30);
    bar.update_settings();

    auto [width3, height3] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    EXPECT_EQ(width3, 102);
    EXPECT_EQ(height3, 18);

    auto [x_pos3, y_pos3] = bar.get_position_size(1920, 1080, width, height);

    EXPECT_EQ(x_pos3, 1920 - 102 - 10);
    EXPECT_EQ(y_pos3, 1080 - 18 - 30);

    config.set(wall::conf::k_lock_bar_alignment, "bottom-center");
    config.set(wall::conf::k_lock_bar_left_padding, 10);
    config.set(wall::conf::k_lock_bar_right_padding, 10);
    config.set(wall::conf::k_lock_bar_top_padding, 10);
    config.set(wall::conf::k_lock_bar_bottom_padding, 15);
    bar.update_settings();

    auto [width4, height4] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    EXPECT_EQ(width4, 102);
    EXPECT_EQ(height4, 18);

    auto [x_pos4, y_pos4] = bar.get_position_size(1920, 1080, width, height);

    EXPECT_EQ(x_pos4, (1920 - 102) / 2);
    EXPECT_EQ(y_pos4, 1080 - 18 - 15);

    config.set(wall::conf::k_lock_bar_alignment, "bottom-left");
    config.set(wall::conf::k_lock_bar_left_padding, 10);
    config.set(wall::conf::k_lock_bar_right_padding, 10);
    config.set(wall::conf::k_lock_bar_top_padding, 10);
    config.set(wall::conf::k_lock_bar_bottom_padding, 10);

    bar.update_settings();

    auto [width5, height5] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    EXPECT_EQ(width5, 102);
    EXPECT_EQ(height5, 18);

    auto [x_pos5, y_pos5] = bar.get_position_size(1920, 1080, width, height);

    EXPECT_EQ(x_pos5, 10);
    EXPECT_EQ(y_pos5, 1080 - 18 - 10);
}

TEST(CairoBarTest, test_bar_draw) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_alignment, "top-left");

    wall::CairoFontCache font_cache{config, CAIRO_SUBPIXEL_ORDER_DEFAULT};
    font_cache.load_font("FiraCode Nerd Font", 12);

    std::string message = "hello world!";

    wall::CairoBarElement bar{config};
    auto [width, height] = bar.get_buffer_size(1920, 1080, 1.0, font_cache, message);

    auto* buffer = (unsigned char*)malloc(4 * width * height);
    auto* surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, width, height, 4 * width);
    auto* cairo = cairo_create(surface);

    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    cairo_set_source_rgb(cairo, 0, 0, 0);
    cairo_paint(cairo);

    bar.draw(cairo, width, height, font_cache, message);
    cairo_surface_write_to_png(surface, "/tmp/test_cairo_bar.png");

    cairo_surface_flush(surface);
    cairo_surface_finish(surface);

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);
    free(buffer);
}
