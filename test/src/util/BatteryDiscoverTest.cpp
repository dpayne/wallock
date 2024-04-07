#include "conf/ConfigDefaultSettings.hpp"
#include "util/BatteryDiscover.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class BatteryDiscoverMock : public wall::BatteryDiscover {
   public:
    BatteryDiscoverMock(const wall::Config& config) : wall::BatteryDiscover(config) {}
    ~BatteryDiscoverMock() override = default;

    [[nodiscard]] auto should_update(std::chrono::time_point<std::chrono::system_clock> now) const -> bool override {
        return wall::BatteryDiscover::should_update(now);
    }
};

class BatteryDiscoverTests : public ::testing::Test {
   public:
    BatteryDiscoverTests() : m_config{wall::Config::get_default_config()} {}

    void SetUp() override {
        m_config = wall::Config::get_default_config();
        m_config.set(wall::conf::k_lock_bar_battery_update_interval_secs, 1);
        m_battery_discover = std::make_unique<BatteryDiscoverMock>(m_config);
    }

    void TearDown() override {}

    wall::Config m_config;
    std::unique_ptr<BatteryDiscoverMock> m_battery_discover;
};

TEST_F(BatteryDiscoverTests, get_battery_info) {
    const auto now = std::chrono::system_clock::now();
    EXPECT_TRUE(m_battery_discover->should_update(now));
    [[maybe_unused]] auto result = m_battery_discover->get_status(now);

    EXPECT_FALSE(m_battery_discover->should_update(now));

    EXPECT_TRUE(m_battery_discover->should_update(now + std::chrono::seconds(2)));
}
