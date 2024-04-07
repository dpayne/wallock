#include <wallock/version.h>
#include <wallock/Wallock.hpp>
#include "conf/Config.hpp"
#include "conf/ConfigDefaultSettings.hpp"
#include "gtest/gtest.h"
#include "util/Formatter.hpp"

TEST(FormatterTests, string_to_module_empty) {
    const auto config = wall::Config::get_default_config();
    wall::Formatter fmter{config};
    std::vector<wall::Module> modules = {wall::Module::Clock, wall::Module::Battery, wall::Module::Network, wall::Module::Keyboard,
                                         wall::Module::CapsLock};

    std::string m_module_separator = ", ";

    auto result = fmter.format(modules, m_module_separator, false, {});
    EXPECT_EQ(result, "");

    result = fmter.format(modules, m_module_separator, false, {{wall::Module::Clock, "replace me!"}});
    EXPECT_EQ(result, "replace me!");

    result = fmter.format(modules, m_module_separator, false, {{wall::Module::Clock, ""}});
    EXPECT_EQ(result, "");

    result = fmter.format(modules, m_module_separator, false, {{wall::Module::Clock, ""}, {wall::Module::Battery, "replace me!"}});
    EXPECT_EQ(result, "replace me!");

    result = fmter.format(modules, m_module_separator, false, {{wall::Module::Clock, ""}, {wall::Module::Battery, ""}});
    EXPECT_EQ(result, "");

    result = fmter.format(modules, m_module_separator, true, {{wall::Module::Clock, "this is a clock"}, {wall::Module::Battery, "battery level"}});
    EXPECT_EQ(result, "this is a clock, battery level");
}

TEST(FormatterTests, format_clock) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_clock_format, "%H:%M:%S");
    config.set(wall::conf::k_lock_indicator_clock_format, "%a, %b %d");

    wall::Formatter fmter{config};

    setenv("TZ", "UTC", 1);
    std::chrono::system_clock::from_time_t(1710000000L);

    auto result = fmter.format_indicator_clock(std::chrono::system_clock::from_time_t(0));
    EXPECT_EQ(result, "Thu, Jan 01");

    result = fmter.format_bar_clock(std::chrono::system_clock::from_time_t(0));
    EXPECT_EQ(result, "00:00:00");

    config.set(wall::conf::k_lock_indicator_clock_format, "");
    result = fmter.format_indicator_clock(std::chrono::system_clock::from_time_t(0));
    EXPECT_EQ(result, "");
}

TEST(FormatterTests, format_battery) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_battery_not_found, "battery not found");
    config.set(wall::conf::k_lock_bar_battery_level_icons, "1,2,3,4,5");
    config.set(wall::conf::k_lock_bar_battery_charging_format, "{icon} {capacity} charging");
    config.set(wall::conf::k_lock_bar_battery_plugged_format, "{icon} {capacity} plugged");
    config.set(wall::conf::k_lock_bar_battery_discharging_format, "{icon} {capacity} discharging");

    wall::Formatter fmter{config};
    auto result = fmter.format_battery({.m_capacity = 42, .m_is_ac_connected = false});
    EXPECT_EQ(result, "3 42 discharging");

    result = fmter.format_battery({.m_capacity = 100, .m_is_ac_connected = true});
    EXPECT_EQ(result, "5 100 plugged");

    result = fmter.format_battery({.m_capacity = 42, .m_is_ac_connected = true});
    EXPECT_EQ(result, "3 42 charging");

    result = fmter.format_battery({.m_capacity = -1, .m_is_ac_connected = true});
    EXPECT_EQ(result, "1 0 charging");

    result = fmter.format_battery({.m_capacity = 110, .m_is_ac_connected = false});
    EXPECT_EQ(result, "5 100 discharging");
}

TEST(FormatterTests, format_keyboard) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_keyboard_format, "{layout} {caps_lock}");
    config.set(wall::conf::k_lock_bar_caps_locked_format, "CAPS!!!");
    config.set(wall::conf::k_lock_bar_caps_unlocked_format, "caps");
    wall::Formatter fmter{config};

    auto result = fmter.format_keyboard("English (US)", true);
    EXPECT_EQ(result, "US CAPS!!!");

    result = fmter.format_keyboard("us", false);
    EXPECT_EQ(result, "us caps");

    result = fmter.format_keyboard("us", true);
    EXPECT_EQ(result, "us CAPS!!!");

    result = fmter.format_keyboard("us (", false);
    EXPECT_EQ(result, "us ( caps");

    result = fmter.format_keyboard("us ()", false);
    EXPECT_EQ(result, "us () caps");

    config.set(wall::conf::k_lock_bar_keyboard_format, "{layout} {caps_lock}");
    config.set(wall::conf::k_lock_bar_caps_locked_format, "");
    config.set(wall::conf::k_lock_bar_caps_unlocked_format, "");
    result = fmter.format_keyboard("", false);
    EXPECT_EQ(result, " ");

    result = fmter.format_keyboard("", true);
    EXPECT_EQ(result, " ");

    config.set(wall::conf::k_lock_bar_keyboard_format, "");
    config.set(wall::conf::k_lock_bar_caps_locked_format, "CAPS!!!");
    config.set(wall::conf::k_lock_bar_caps_unlocked_format, "caps");
    result = fmter.format_keyboard("us", false);
    EXPECT_EQ(result, "");
}

TEST(FormatterTests, format_network) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_lock_bar_network_format_disconnected, "disconnected");
    config.set(wall::conf::k_lock_bar_network_format_wifi, "wifi {ifname} {ipaddr}");
    config.set(wall::conf::k_lock_bar_network_format_ethernet, "ethernet {ipaddr}");
    config.set(wall::conf::k_lock_bar_network_format_ifname_max_length, 50);

    wall::Formatter fmter{config};

    wall::Network network;
    network.m_ipv4_address = "42.42.42.42";
    network.m_is_wireless = false;
    network.m_name = "eth0";
    auto result = fmter.format_network(network);
    EXPECT_EQ(result, "ethernet 42.42.42.42");

    network.m_ipv4_address = "";
    network.m_ipv6_address = "42:42:42:42";
    network.m_is_wireless = true;
    network.m_name = "wlan0";
    result = fmter.format_network(network);
    EXPECT_EQ(result, "wifi wlan0 42:42:42:42");

    network.m_ipv4_address = "";
    network.m_ipv6_address = "";
    network.m_is_wireless = false;
    network.m_name = "eth0";
    result = fmter.format_network(network);
    EXPECT_EQ(result, "disconnected");

    config.set(wall::conf::k_lock_bar_network_format_ifname_max_length, 2);
    network.m_ipv4_address = "42.42.42.42";
    network.m_is_wireless = true;
    network.m_name = "wlan0";
    result = fmter.format_network(network);
    EXPECT_EQ(result, "wifi wl 42.42.42.42");

    config.set(wall::conf::k_lock_bar_network_format_ifname_max_length, 50);
    config.set(wall::conf::k_lock_bar_network_format_wifi, "");
    network.m_ipv4_address = "42.42.42.42";
    network.m_is_wireless = true;
    network.m_name = "wlan0";
    result = fmter.format_network(network);
    EXPECT_EQ(result, "");
}
