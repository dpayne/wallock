#include <cairo.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "conf/ConfigDefaultSettings.hpp"
#include "overlay/CairoIndicatorElement.hpp"

TEST(CairoIndicatorTest, test_cairo_indicator) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");

    config.set(wall::conf::k_lock_indicator_ring_inner_fill_color_input, "#888888AA");

    const auto ring_radius = wall_conf_get(config, lock_indicator, ring_radius);
    const auto ring_thickness = wall_conf_get(config, lock_indicator, ring_thickness);
    const auto width = ring_radius * 2.0 + ring_thickness * 2.0 + 10.0;
    const auto height = width;

    auto* buffer = (unsigned char*)malloc(4 * width * height);
    auto* surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, width, height, 4 * width);
    auto* cairo = cairo_create(surface);

    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    const auto center_x = width / 2.0;
    const auto center_y = height / 2.0;

    wall::CairoIndicatorElement indicator{config};

    // paint black background
    cairo_set_source_rgb(cairo, 0, 0, 0);
    cairo_paint(cairo);

    indicator.on_state_change(wall::State::Input);
    indicator.draw(cairo, center_x, center_y);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_input.png");

    indicator.on_state_change(wall::State::Keypress);
    indicator.draw(cairo, center_x, center_y);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_keypress.png");

    indicator.on_state_change(wall::State::Backspace);
    indicator.draw(cairo, center_x, center_y);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_backspace.png");

    indicator.on_state_change(wall::State::Cleared);
    indicator.draw(cairo, center_x, center_y);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_cleared.png");

    indicator.on_state_change(wall::State::Verifying);
    indicator.draw(cairo, center_x, center_y);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_verifying.png");

    indicator.on_state_change(wall::State::Wrong);
    indicator.draw(cairo, center_x, center_y);
    cairo_surface_write_to_png(surface, "/tmp/test_indicator_wrong.png");

    cairo_surface_flush(surface);
    cairo_surface_finish(surface);

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);

    free(buffer);
}
