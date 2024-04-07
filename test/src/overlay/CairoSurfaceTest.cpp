#include <gtest/gtest.h>
#include <wayland-client-protocol.h>

#include "MockObjects.hpp"
#include "State.hpp"
#include "TestUtils.hpp"
#include "overlay/CairoSurface.hpp"

class CairoSurfaceMock : public wall::CairoSurface {
   public:
    CairoSurfaceMock(const wall::Config& config, wall::Surface* surface) : wall::CairoSurface(config, surface, WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB) {}

    auto draw_frame([[maybe_unused]] int32_t width, [[maybe_unused]] int32_t height, [[maybe_unused]] int32_t scale)
        -> std::chrono::milliseconds override {
        set_last_draw_time(get_now());
        m_draw_count++;
        return std::chrono::milliseconds{100};
    }

    [[nodiscard]] auto get_draw_count() const -> uint32_t { return m_draw_count; }

    [[nodiscard]] auto get_now() const -> std::chrono::time_point<std::chrono::system_clock> { return wall::CairoSurface::get_now(); }

    [[nodiscard]] auto get_state() const -> wall::State { return wall::CairoSurface::get_state(); }

    [[nodiscard]] auto get_cairo_state() const -> wall::CairoState* { return wall::CairoSurface::get_cairo_state(); }

    [[nodiscard]] auto get_last_draw_time() const -> std::chrono::time_point<std::chrono::system_clock> {
        return wall::CairoSurface::get_last_draw_time();
    }

    [[nodiscard]] auto get_last_activity_time() const -> std::chrono::time_point<std::chrono::system_clock> {
        return wall::CairoSurface::get_last_activity_time();
    }

    auto set_last_draw_time(std::chrono::time_point<std::chrono::system_clock> now) -> void { wall::CairoSurface::set_last_draw_time(now); }

    auto set_idle_timeout(std::chrono::milliseconds idle_timeout) -> void { wall::CairoSurface::set_idle_timeout(idle_timeout); }

    auto set_last_activity_time(std::chrono::time_point<std::chrono::system_clock> now) -> void { wall::CairoSurface::set_last_activity_time(now); }

    auto set_state(wall::State state) -> void { wall::CairoSurface::set_state(state); }

    auto set_is_visible_on_idle(bool is_visible_on_idle) -> void { wall::CairoSurface::set_is_visible_on_idle(is_visible_on_idle); }

    auto should_draw_frame_on_idle(wl_surface* surface, wl_subsurface* subsurface) -> bool {
        return wall::CairoSurface::should_draw_frame_on_idle(surface, subsurface);
    }

    auto set_now(std::chrono::time_point<std::chrono::system_clock> now) -> void { wall::CairoSurface::set_now(now); }

   private:
    uint32_t m_draw_count{};
};

TEST(CairoSurfaceTest, idle_timeout) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");

    wall::Loop loop;
    RegistryMock registry_mock{config, &loop};
    SurfaceMock surface_mock{config, nullptr, &registry_mock};
    CairoSurfaceMock surface{config, &surface_mock};
    surface.set_idle_timeout(std::chrono::milliseconds{1000});
    surface.set_is_visible_on_idle(false);

    surface.set_state(wall::State::Input);
    surface.draw(1920, 1080, 1);

    surface.on_state_change(wall::State::Keypress);
    surface.set_now(surface.get_now() + std::chrono::milliseconds{500});

    ASSERT_TRUE(surface.should_draw_frame_on_idle(nullptr, nullptr));
    surface.set_now(surface.get_now() + std::chrono::milliseconds{600});
    ASSERT_FALSE(surface.get_state() == wall::State::Idle);

    ASSERT_FALSE(surface.should_draw_frame_on_idle(nullptr, nullptr));
    ASSERT_EQ(surface.get_state(), wall::State::Idle);

    surface.set_is_visible_on_idle(true);
    surface.on_state_change(wall::State::Keypress);
    surface.set_now(surface.get_now() + std::chrono::milliseconds{2000});
    ASSERT_TRUE(surface.should_draw_frame_on_idle(nullptr, nullptr));
    ASSERT_EQ(surface.get_state(), wall::State::Idle);
}

TEST(CairoSurfaceTest, invalid_size) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");

    wall::Loop loop;
    RegistryMock registry_mock{config, &loop};
    SurfaceMock surface_mock{config, nullptr, &registry_mock};
    CairoSurfaceMock surface{config, &surface_mock};
    surface.draw(0, 0, 1);
    ASSERT_EQ(surface.get_draw_count(), 0);
}

TEST(CairoSurfaceTest, activity_set) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_monitor, "all");
    config.set(wall::conf::k_lock_indicator_monitor, "all");

    wall::Loop loop;
    RegistryMock registry_mock{config, &loop};
    SurfaceMock surface_mock{config, nullptr, &registry_mock};
    CairoSurfaceMock surface{config, &surface_mock};

    const auto time = wall::TestUtils::convert_date_string_to_time_point("2001-01-01 12:00:00");
    surface.set_now(time);

    surface.on_state_change(wall::State::Idle);
    ASSERT_FALSE(surface.get_last_activity_time() == time);

    surface.on_state_change(wall::State::Keypress);
    ASSERT_TRUE(surface.get_last_activity_time() == time);

    surface.set_now(time + std::chrono::milliseconds{1000});
    surface.on_state_change(wall::State::Backspace);
    ASSERT_TRUE(surface.get_last_activity_time() == (time + std::chrono::milliseconds{1000}));

    surface.set_now(time + std::chrono::milliseconds{5000});
    surface.on_state_change(wall::State::Wrong);
    ASSERT_TRUE(surface.get_last_activity_time() == (time + std::chrono::milliseconds{5000}));

    surface.set_now(time + std::chrono::milliseconds{10000});
    surface.on_state_change(wall::State::Cleared);
    ASSERT_TRUE(surface.get_last_activity_time() == (time + std::chrono::milliseconds{5000}));
    ASSERT_EQ(surface.get_state(), wall::State::Cleared);
}
