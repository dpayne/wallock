#include "conf/Config.hpp"
#include "conf/ConfigDefaultSettings.hpp"
#include "util/NetworkDiscover.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class NetworkDiscoverMock : public wall::NetworkDiscover {
   public:
    NetworkDiscoverMock(const wall::Config& config) : wall::NetworkDiscover(config) {}
    ~NetworkDiscoverMock() override = default;

    [[nodiscard]] auto should_update(std::chrono::time_point<std::chrono::system_clock> now) const -> bool override {
        return wall::NetworkDiscover::should_update(now);
    }
};

class NetworkDiscoverTests : public ::testing::Test {
   public:
    NetworkDiscoverTests() : m_config{wall::Config::get_default_config()} {}
    ~NetworkDiscoverTests() override = default;

    void SetUp() override {
        m_config = wall::Config::get_default_config();
        m_config.set(wall::conf::k_lock_bar_network_update_interval_secs, 1);
        m_network_discover = std::make_unique<NetworkDiscoverMock>(m_config);
    }

    void TearDown() override {}

    wall::Config m_config;
    std::unique_ptr<NetworkDiscoverMock> m_network_discover;
};

TEST_F(NetworkDiscoverTests, get_Network_info) {
    const auto now = std::chrono::system_clock::now();
    EXPECT_TRUE(m_network_discover->should_update(now));
    auto result = m_network_discover->get_status(now);

    EXPECT_TRUE(!result.m_ipv4_address.empty() || !result.m_ipv6_address.empty());
    EXPECT_TRUE(!result.m_name.empty());
    EXPECT_FALSE(m_network_discover->should_update(now));
    EXPECT_TRUE(m_network_discover->should_update(now + std::chrono::seconds(2)));
}
