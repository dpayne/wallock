#include <gtest/gtest.h>

#include "MockObjects.hpp"
#include "TestUtils.hpp"
#include "conf/ConfigDefaultSettings.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "overlay/CairoBarSurface.hpp"
#include "registry/BufferPool.hpp"
#include "surface/Surface.hpp"
#include "util/Loop.hpp"

class CairoBarSurfaceMock : public wall::CairoBarSurface {
   public:
    CairoBarSurfaceMock(const wall::Config& config, wall::Surface* surface)
        : wall::CairoBarSurface(config, surface, WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB) {}

    ~CairoBarSurfaceMock() override = default;

    auto update_surface([[maybe_unused]] wl_surface* child_surface,
                        [[maybe_unused]] wl_subsurface* subsurface,
                        [[maybe_unused]] int32_t subsurf_xpos,
                        [[maybe_unused]] int32_t subsurf_ypos,
                        [[maybe_unused]] wall::Buffer* buffer) -> void override {
        m_last_subpos_x = subsurf_xpos;
        m_last_subpos_y = subsurf_ypos;
        m_draw_count++;
    }

    [[nodiscard]] auto generate_message() -> std::string override { return wall::CairoBarSurface::generate_message(); }

    auto set_now(std::chrono::time_point<std::chrono::system_clock> now) -> void { wall::CairoBarSurface::set_now(now); }
    uint32_t m_last_subpos_x{};
    uint32_t m_last_subpos_y{};
    uint32_t m_draw_count{};
};

TEST(CairoBarSurfaceTest, test_bar_generate_modules) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    wall::Loop loop;
    KeyboardMock keyboard{&loop};
    keyboard.m_layout = "us";
    keyboard.m_is_caps_lock = false;
    RegistryMock registry{config, &loop};
    SeatMock seat{&loop};
    seat.set_keyboard(&keyboard);
    registry.set_seat(&seat);
    SurfaceMock surface{config, nullptr, &registry};
    CairoBarSurfaceMock bar_surface{config, &surface};
    bar_surface.set_now(wall::TestUtils::convert_date_string_to_time_point("2021-01-01 00:00:00"));

    auto message = bar_surface.generate_message();
    EXPECT_EQ(message, R"(us 󰌓  󰈀  Fri, Jan 01)");

    config.set(wall::conf::k_lock_bar_modules, "keyboard");
    bar_surface.update_settings();
    message = bar_surface.generate_message();
    EXPECT_EQ(message, R"(us 󰌓)");

    config.set(wall::conf::k_lock_bar_modules, "keyboard, network");
    bar_surface.update_settings();
    message = bar_surface.generate_message();
    EXPECT_EQ(message, R"(us 󰌓  󰈀)");

    config.set(wall::conf::k_lock_bar_modules, "keyboard, network, battery, caps_lock, clock");
}

TEST(CairoBarSurfaceTest, test_should_draw) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_enabled, false);
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");

    BufferPoolMock buffer_pool;

    wall::Loop loop;
    KeyboardMock keyboard{&loop};
    keyboard.m_layout = "us";
    keyboard.m_is_caps_lock = false;
    RegistryMock registry{config, &loop};
    SeatMock seat{&loop};
    seat.set_keyboard(&keyboard);
    registry.set_seat(&seat);
    registry.set_buffer_pool(&buffer_pool);
    SurfaceMock surface{config, nullptr, &registry};
    CairoBarSurfaceMock bar_surface{config, &surface};
    bar_surface.on_state_change(wall::State::Input);
    bar_surface.set_now(wall::TestUtils::convert_date_string_to_time_point("2021-01-01 00:00:00"));

    bar_surface.draw(1920, 1080, 1);
    EXPECT_EQ(bar_surface.m_draw_count, 0);

    config.set(wall::conf::k_lock_bar_enabled, true);
    config.set(wall::conf::k_wallpaper_bar_enabled, false);
    surface.m_resource_mode = wall::ResourceMode::Wallpaper;
    bar_surface.update_settings();
    bar_surface.draw(1920, 1080, 1);
    EXPECT_EQ(bar_surface.m_draw_count, 0);

    config.set(wall::conf::k_lock_bar_enabled, false);
    config.set(wall::conf::k_wallpaper_bar_enabled, true);
    surface.m_resource_mode = wall::ResourceMode::Wallpaper;
    bar_surface.update_settings();
    bar_surface.draw(1920, 1080, 1);
    EXPECT_EQ(bar_surface.m_draw_count, 1);
    EXPECT_NEAR(bar_surface.m_last_subpos_x, 814, 1);
    EXPECT_NEAR(bar_surface.m_last_subpos_y, 1041, 1);
}
