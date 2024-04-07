#pragma once

#include <libudev.h>
#include <chrono>
#include <cstdint>
#include <optional>
#include "conf/Config.hpp"

namespace wall {
struct BatteryStatus {
    std::optional<int32_t> m_capacity;
    std::optional<bool> m_is_ac_connected;
};
class BatteryDiscover {
   public:
    BatteryDiscover(const Config& config);

    virtual ~BatteryDiscover();

    auto get_status(std::chrono::time_point<std::chrono::system_clock> now) -> const BatteryStatus&;

   protected:
    [[nodiscard]] auto get_battery_capacity(struct udev* udev) const -> std::optional<int32_t>;

    [[nodiscard]] auto is_ac_connected(struct udev* udev) const -> std::optional<bool>;

    [[nodiscard]] virtual auto should_update(std::chrono::time_point<std::chrono::system_clock> now) const -> bool;

    [[nodiscard]] auto get_config() const -> const Config&;

   private:
    const Config& m_config;

    udev* m_udev{};

    BatteryStatus m_last_status{};
    std::chrono::time_point<std::chrono::system_clock> m_last_update{};
    std::chrono::seconds m_update_interval{};
};
}  // namespace wall
