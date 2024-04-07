#pragma once

#include <chrono>
#include <string>
#include "conf/Config.hpp"

namespace wall {
struct Network {
    bool m_is_wireless{false};
    std::string m_name;
    std::string m_ipv4_address;
    std::string m_ipv6_address;
};

class NetworkDiscover {
   public:
    NetworkDiscover(const Config& config);
    virtual ~NetworkDiscover();
    auto get_status(std::chrono::time_point<std::chrono::system_clock> now) -> const Network&;

   protected:
    [[nodiscard]] virtual auto should_update(std::chrono::time_point<std::chrono::system_clock> now) const -> bool;

    [[nodiscard]] auto get_network() const -> Network;

    auto get_network_from_interface(const char* ifname) const -> Network;

    auto is_wireless(const char* ifname) const -> bool;

    auto get_ipv4_address(const char* ifname) const -> std::string;

    auto get_ipv6_address(const char* ifname) const -> std::string;

    [[nodiscard]] auto get_config() const -> const Config&;

   private:
    const Config& m_config;

    Network m_network;

    std::chrono::time_point<std::chrono::system_clock> m_last_update{};
    std::chrono::seconds m_update_interval{};
};
}  // namespace wall
