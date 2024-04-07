#include <chrono>
#include <util/NetworkDiscover.hpp>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include "conf/ConfigMacros.hpp"
#include "util/Log.hpp"

wall::NetworkDiscover::NetworkDiscover(const Config& config) : m_config{config} {
    m_update_interval = std::chrono::seconds(wall_conf_get(get_config(), lock_bar, network_update_interval_secs));
}

wall::NetworkDiscover::~NetworkDiscover() = default;

auto wall::NetworkDiscover::get_config() const -> const Config& { return m_config; }

auto wall::NetworkDiscover::should_update(std::chrono::time_point<std::chrono::system_clock> now) const -> bool {
    return (now - m_last_update) > m_update_interval;
}

auto wall::NetworkDiscover::get_network_from_interface(const char* ifname) const -> Network {
    Network network;
    network.m_name = ifname;
    network.m_ipv4_address = get_ipv4_address(ifname);
    network.m_ipv6_address = get_ipv6_address(ifname);
    network.m_is_wireless = is_wireless(ifname);

    return network;
}

auto wall::NetworkDiscover::is_wireless(const char* ifname) const -> bool {
    int sock = -1;
    char protocol[IFNAMSIZ] = {0};  // NOLINT
    struct iwreq pwrq;
    memset(&pwrq, 0, sizeof(pwrq));
    strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("socket: {}", strerror(errno));
        return false;
    }

    if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
        strncpy(protocol, pwrq.u.name, IFNAMSIZ);
        close(sock);
        return true;
    }

    close(sock);
    return false;
}

auto wall::NetworkDiscover::get_ipv6_address(const char* ifname) const -> std::string {
    int32_t sock = -1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("Failed to open socket {}", strerror(errno));
        return "";
    }

    struct ifreq ifr6;
    memset(&ifr6, 0, sizeof(ifr6));
    strncpy(ifr6.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFADDR, &ifr6) == -1) {
        LOG_DEBUG("Failed to get ipv6 address of interface {}", strerror(errno));
    } else {
        char ipv6[INET6_ADDRSTRLEN];  // NOLINT
        inet_ntop(AF_INET6, &((struct sockaddr_in6*)&ifr6.ifr_addr)->sin6_addr, ipv6, INET6_ADDRSTRLEN);

        auto result = std::string{ipv6};
        if (result == "::" || result == "::1") {
            result = "";
        }
    }

    close(sock);
    return "";
}

auto wall::NetworkDiscover::get_ipv4_address(const char* ifname) const -> std::string {
    int32_t sock = -1;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        LOG_ERROR("socket: {}", strerror(errno));
        return "";
    }

    // get ipv4 address of interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    std::string result;
    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        LOG_DEBUG("Failed to get ipv4 address of interface {}", strerror(errno));
    } else {
        char ipv4[INET_ADDRSTRLEN];  // NOLINT
        inet_ntop(AF_INET, &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr, ipv4, INET_ADDRSTRLEN);

        result = std::string{ipv4};
        if (result == "127.0.0.1") {
            result = "";
        }
    }

    close(sock);
    return result;
}

auto wall::NetworkDiscover::get_network() const -> Network {
    struct ifaddrs* ifaddr{nullptr};
    struct ifaddrs* ifa{nullptr};

    if (getifaddrs(&ifaddr) == -1) {
        LOG_ERROR("Failed to get interface addresses: {}", strerror(errno));
        return {};
    }

    Network network;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || (ifa->ifa_addr->sa_family != AF_PACKET && ifa->ifa_addr->sa_family != AF_INET6)) {
            continue;
        }

        network = get_network_from_interface(ifa->ifa_name);
        if (!network.m_ipv4_address.empty() || !network.m_ipv6_address.empty()) {
            break;
        }
    }

    freeifaddrs(ifaddr);
    return network;
}

auto wall::NetworkDiscover::get_status(std::chrono::time_point<std::chrono::system_clock> now) -> const wall::Network& {
    if (should_update(now)) {
        m_network = get_network();
        m_last_update = now;
    }

    return m_network;
}
