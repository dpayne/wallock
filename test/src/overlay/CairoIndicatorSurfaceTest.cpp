#include <gtest/gtest.h>

#include "MockObjects.hpp"
#include "TestUtils.hpp"
#include "conf/ConfigDefaultSettings.hpp"
#include "overlay/CairoIndicatorMessage.hpp"
#include "overlay/CairoIndicatorSurface.hpp"
#include "util/Loop.hpp"

class CairoIndicatorSurfaceMock : public wall::CairoIndicatorSurface {
   public:
    CairoIndicatorSurfaceMock(const wall::Config& config, wall::Surface* surface)
        : wall::CairoIndicatorSurface(config, surface, WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB) {}

    ~CairoIndicatorSurfaceMock() override = default;

    auto update_surface([[maybe_unused]] wl_surface* child_surface,
                        [[maybe_unused]] wl_subsurface* subsurface,
                        [[maybe_unused]] int32_t subsurf_xpos,
                        [[maybe_unused]] int32_t subsurf_ypos,
                        [[maybe_unused]] wall::Buffer* buffer) -> void override {
        m_last_subpos_x = subsurf_xpos;
        m_last_subpos_y = subsurf_ypos;
        m_draw_count++;
    }

    auto set_now(std::chrono::time_point<std::chrono::system_clock> now) -> void { wall::CairoIndicatorSurface::set_now(now); }

    auto get_buffer_size(int32_t buffer_diameter) -> std::pair<int32_t, int32_t> override {
        return wall::CairoIndicatorSurface::get_buffer_size(buffer_diameter);
    }

    auto get_indicator_message() -> const wall::CairoIndicatorMessage& { return wall::CairoIndicatorSurface::get_indicator_message(); }

    auto update_message() -> void { wall::CairoIndicatorSurface::update_message(); }

    uint32_t m_last_subpos_x{};
    uint32_t m_last_subpos_y{};
    uint32_t m_draw_count{};
};

TEST(CairoIndicatorSurfaceTest, test_should_draw) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_enabled, false);

    wall::Loop loop;
    BufferPoolMock buffer_pool;
    KeyboardMock keyboard{&loop};
    keyboard.m_layout = "us";
    keyboard.m_is_caps_lock = false;
    RegistryMock registry{config, &loop};
    SeatMock seat{&loop};
    seat.set_keyboard(&keyboard);
    registry.set_seat(&seat);
    registry.set_buffer_pool(&buffer_pool);
    SurfaceMock surface{config, nullptr, &registry};
    CairoIndicatorSurfaceMock indicator_surface{config, &surface};
    indicator_surface.on_state_change(wall::State::Keypress);

    indicator_surface.draw(1920, 1080);
    EXPECT_EQ(indicator_surface.m_draw_count, 0);

    config.set(wall::conf::k_lock_indicator_enabled, true);
    indicator_surface.update_settings();
    indicator_surface.on_state_change(wall::State::Keypress);
    indicator_surface.draw(1920, 1080);
    EXPECT_EQ(indicator_surface.m_draw_count, 1);
    EXPECT_EQ(indicator_surface.m_last_subpos_x, 877);
    EXPECT_EQ(indicator_surface.m_last_subpos_y, 455);
}

TEST(CairoIndicatorSurfaceTest, test_caps_lock) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_enabled, true);
    config.set(wall::conf::k_lock_indicator_message_caps_lock, "CAPS LOCK");

    wall::Loop loop;
    BufferPoolMock buffer_pool;
    KeyboardMock keyboard{&loop};
    keyboard.m_layout = "us";
    keyboard.m_is_caps_lock = true;
    RegistryMock registry{config, &loop};
    SeatMock seat{&loop};
    seat.set_keyboard(&keyboard);
    registry.set_seat(&seat);
    registry.set_buffer_pool(&buffer_pool);
    SurfaceMock surface{config, nullptr, &registry};
    CairoIndicatorSurfaceMock indicator_surface{config, &surface};
    indicator_surface.on_state_change(wall::State::Keypress);

    indicator_surface.draw(1920, 1080);
    EXPECT_EQ(indicator_surface.m_draw_count, 1);
    EXPECT_EQ(indicator_surface.get_indicator_message().get_message(), "CAPS LOCK");
}

TEST(CairoIndicatorSurfaceTest, buffer_size) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");
    config.set(wall::conf::k_lock_indicator_enabled, true);
    config.set(wall::conf::k_lock_indicator_message_caps_lock,
               "this is a super long message that should force the buffer to be larger than the default size");

    wall::Loop loop;
    BufferPoolMock buffer_pool;
    KeyboardMock keyboard{&loop};
    keyboard.m_layout = "us";
    keyboard.m_is_caps_lock = false;
    RegistryMock registry{config, &loop};
    SeatMock seat{&loop};
    seat.set_keyboard(&keyboard);
    registry.set_seat(&seat);
    registry.set_buffer_pool(&buffer_pool);
    SurfaceMock surface{config, nullptr, &registry};
    CairoIndicatorSurfaceMock indicator_surface{config, &surface};
    indicator_surface.on_state_change(wall::State::Keypress);
    indicator_surface.update_message();

    auto radius = wall_conf_get(config, lock_indicator, ring_radius);
    auto thickness = wall_conf_get(config, lock_indicator, ring_thickness);
    auto buffer_diameter = (radius + thickness) * 2;
    auto [width, height] = indicator_surface.get_buffer_size(buffer_diameter);

    EXPECT_EQ(width, buffer_diameter);
    EXPECT_EQ(height, buffer_diameter);

    keyboard.m_is_caps_lock = true;
    indicator_surface.update_message();
    auto [width2, height2] = indicator_surface.get_buffer_size(buffer_diameter);

    EXPECT_EQ(width2, 1377);
    EXPECT_EQ(height2, buffer_diameter);
}
