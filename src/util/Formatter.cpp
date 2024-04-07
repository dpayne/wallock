#include "util/Formatter.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <ctime>
#include <optional>
#include <utility>

#include "conf/ConfigMacros.hpp"
#include "util/BatteryDiscover.hpp"
#include "util/NetworkDiscover.hpp"
#include "util/StringUtils.hpp"

namespace wall {
class Config;
}  // namespace wall
struct tm;

wall::Formatter::Formatter(const Config& config) : m_config(config) {
    m_module_to_string = {
        {Module::None, "none"},       {Module::Clock, "clock"},       {Module::Battery, "battery"},
        {Module::Network, "network"}, {Module::Keyboard, "keyboard"}, {Module::CapsLock, "caps_lock"},
    };

    for (const auto& [key, value] : m_module_to_string) {
        m_string_to_module[value] = key;
    }
}

auto wall::Formatter::from_string_to_module(const std::string& str) -> wall::Module {
    const auto iter = m_string_to_module.find(str);
    if (iter != m_string_to_module.end()) {
        return iter->second;
    }

    return Module::None;
}

auto wall::Formatter::to_string(const wall::Module& mod) -> std::string {
    const auto iter = m_module_to_string.find(mod);
    if (iter != m_module_to_string.end()) {
        return iter->second;
    }

    return "";
}

auto wall::Formatter::get_config() const -> const Config& { return m_config; }

auto wall::Formatter::format(const std::vector<Module>& modules,
                             const std::string& module_separator,
                             bool is_draw_on_empty,
                             const std::map<Module, std::string>& replacements) const -> std::string {
    std::string str;

    for (const auto& mod : modules) {
        const auto iter = replacements.find(mod);
        if (iter != replacements.end()) {
            const auto& replacement = iter->second;
            if (!replacement.empty() || is_draw_on_empty) {
                if (!str.empty()) {
                    str += module_separator;
                }
                str += replacement;
            }
        }
    }

    return str;
}

auto wall::Formatter::format(const std::string& format_template, const std::map<std::string, std::string>& replacements) const -> std::string {
    std::string formatted = format_template;

    for (const auto& [key, value] : replacements) {
        std::string search = "{" + key + "}";
        auto pos = formatted.find(search);
        if (pos != std::string::npos) {
            formatted.replace(pos, search.size(), value);
        }
    }

    return formatted;
}

auto wall::Formatter::format_keyboard(const std::string& layout, bool is_caps_lock) const -> std::string {
    auto str = layout;
    if (wall_conf_get(get_config(), lock_bar, layout_guess_short_name) && layout.find('(') != std::string::npos &&
        layout.find(')') != std::string::npos) {
        auto substr = layout.substr(layout.find('(') + 1, layout.find(')') - layout.find('(') - 1);
        if (!substr.empty()) {
            str = substr;
        }
    } else {
        str = layout;
    }

    const auto format_str = std::string{wall_conf_get(get_config(), lock_bar, keyboard_format)};
    std::map<std::string, std::string> replacements = {
        {"layout", str},
        {"caps_lock", format_caps_lock(is_caps_lock)},
    };

    return format(format_str, replacements);
}

auto wall::Formatter::format_network(const Network& network) const -> std::string {
    std::string format_str;
    if (network.m_ipv4_address.empty() && network.m_ipv6_address.empty()) {
        format_str = wall_conf_get(get_config(), lock_bar, network_format_disconnected);
    } else if (network.m_is_wireless) {
        format_str = wall_conf_get(get_config(), lock_bar, network_format_wifi);
    } else {
        format_str = wall_conf_get(get_config(), lock_bar, network_format_ethernet);
    }

    const auto name = network.m_name.substr(0, wall_conf_get(get_config(), lock_bar, network_format_ifname_max_length));
    std::map<std::string, std::string> replacements = {
        {"ifname", name},
        {"ipaddr", network.m_ipv4_address.empty() ? network.m_ipv6_address : network.m_ipv4_address},
    };

    return format(format_str, replacements);
}

auto wall::Formatter::format_battery(const BatteryStatus& status) const -> std::string {
    if (!status.m_capacity.has_value()) {
        return std::string{wall_conf_get(get_config(), lock_bar, battery_not_found)};
    }

    auto pick_icon = [](const std::vector<std::string>& icons, int32_t capacity) -> std::string {
        if (icons.empty() || capacity < 0 || capacity > 100) {
            return {};
        }

        if (capacity == 0) {
            return icons.front();
        }

        const auto index = (capacity - 1) / (100 / icons.size());
        return icons[index];
    };

    const auto capacity = std::clamp(status.m_capacity.value_or(0), 0, 100);
    const auto is_ac_connected = status.m_is_ac_connected.value_or(false);
    const auto battery_level_icons_str = wall_conf_get(get_config(), lock_bar, battery_level_icons);
    const auto battery_level_icons = StringUtils::split_and_trim(std::string{battery_level_icons_str}, ',');
    const auto battery_level_icon = pick_icon(battery_level_icons, capacity);

    std::string format_str;
    if (is_ac_connected && capacity < 100) {
        format_str = wall_conf_get(get_config(), lock_bar, battery_charging_format);
    } else if (is_ac_connected) {
        format_str = wall_conf_get(get_config(), lock_bar, battery_plugged_format);
    } else {
        format_str = wall_conf_get(get_config(), lock_bar, battery_discharging_format);
    }

    std::map<std::string, std::string> replacements = {
        {"capacity", std::to_string(capacity)},
        {"icon", battery_level_icon},
    };
    return format(std::string{format_str}, replacements);
}
auto wall::Formatter::format_bar_clock(std::chrono::time_point<std::chrono::system_clock> now) const -> std::string {
    return format_clock(now, wall_conf_get(get_config(), lock_bar, clock_format));
}

auto wall::Formatter::format_indicator_clock(std::chrono::time_point<std::chrono::system_clock> now) const -> std::string {
    return format_clock(now, wall_conf_get(get_config(), lock_indicator, clock_format));
}

auto wall::Formatter::format_clock(std::chrono::time_point<std::chrono::system_clock> now, std::string_view format_str) const -> std::string {
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm time_struct = *std::localtime(&time);
    std::array<char, 80> buffer;

    // this avoids a warning about strftime being unsafe
    std::size_t (*strftime)(char*, std::size_t, const char*, const std::tm*) = nullptr;
    strftime = std::strftime;
    strftime(buffer.data(), buffer.size(), format_str.data(), &time_struct);
    return std::string{buffer.data()};
}

auto wall::Formatter::format_caps_lock(bool is_caps_lock) const -> std::string {
    if (is_caps_lock) {
        return std::string{wall_conf_get(get_config(), lock_bar, caps_locked_format)};
    }

    return std::string{wall_conf_get(get_config(), lock_bar, caps_unlocked_format)};
}
