#include "util/BatteryDiscover.hpp"

#include <libudev.h>
#include <spdlog/common.h>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "conf/ConfigMacros.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::BatteryDiscover::BatteryDiscover(const Config& config) : m_config(config), m_udev(udev_new()) {
    if (m_udev == nullptr) {
        LOG_FATAL("Cannot create udev context.");
    }
    m_update_interval = std::chrono::seconds(wall_conf_get(get_config(), lock_bar, battery_update_interval_secs));
}

wall::BatteryDiscover::~BatteryDiscover() { udev_unref(m_udev); }

auto wall::BatteryDiscover::should_update(std::chrono::time_point<std::chrono::system_clock> now) const -> bool {
    const auto diff = now - m_last_update;
    return diff > m_update_interval;
}

auto wall::BatteryDiscover::get_config() const -> const Config& { return m_config; }

auto wall::BatteryDiscover::get_status(std::chrono::time_point<std::chrono::system_clock> now) -> const BatteryStatus& {
    if (should_update(now)) {
        m_last_status = BatteryStatus{get_battery_capacity(m_udev), is_ac_connected(m_udev)};
        m_last_update = now;
    }

    return m_last_status;
}

auto wall::BatteryDiscover::get_battery_capacity(struct udev* udev) const -> std::optional<int32_t> {
    // Create a list of the devices in the 'power_supply' subsystem.
    auto* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "power_supply");
    udev_enumerate_scan_devices(enumerate);
    auto* devices = udev_enumerate_get_list_entry(enumerate);

    // For each item enumerated, check if it's a battery.
    struct udev_list_entry* entry{};
    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* dev{};
        dev = udev_device_new_from_syspath(udev, path);

        if (strcmp(udev_device_get_sysattr_value(dev, "type"), "Battery") == 0) {
            const auto* capacity_str = udev_device_get_sysattr_value(dev, "capacity");
            if (capacity_str != nullptr) {
                const auto capacity = atoi(capacity_str);
                udev_device_unref(dev);
                udev_enumerate_unref(enumerate);
                return capacity;
            }
        }

        udev_device_unref(dev);
    }

    // Cleanup
    udev_enumerate_unref(enumerate);
    return std::nullopt;
}

auto wall::BatteryDiscover::is_ac_connected(struct udev* udev) const -> std::optional<bool> {
    auto* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "power_supply");
    udev_enumerate_scan_devices(enumerate);
    auto* devices = udev_enumerate_get_list_entry(enumerate);

    struct udev_list_entry* entry{};
    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* dev;
        dev = udev_device_new_from_syspath(udev, path);

        if (strcmp(udev_device_get_sysattr_value(dev, "type"), "Mains") == 0) {
            const auto online = atoi(udev_device_get_sysattr_value(dev, "online"));
            udev_device_unref(dev);
            udev_enumerate_unref(enumerate);
            return online != 0;  // NOLINT
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    return std::nullopt;
}
