#pragma once

#include <map>
#include <string>
#include "conf/Config.hpp"
#include "util/BatteryDiscover.hpp"
#include "util/NetworkDiscover.hpp"

namespace wall {
enum class Module {
    None,
    Clock,
    Battery,
    Network,
    Keyboard,
    CapsLock,
};

class Formatter {
   public:
    Formatter(const Config& config);
    virtual ~Formatter() = default;

    [[nodiscard]] auto from_string_to_module(const std::string& str) -> wall::Module;

    [[nodiscard]] auto to_string(const wall::Module& mod) -> std::string;

    [[nodiscard]] auto format_keyboard(const std::string& layout, bool is_caps_lock) const -> std::string;

    [[nodiscard]] auto format_network(const Network& network) const -> std::string;

    [[nodiscard]] auto format_battery(const BatteryStatus& status) const -> std::string;

    [[nodiscard]] auto format_caps_lock(bool is_caps_lock) const -> std::string;

    [[nodiscard]] auto format_bar_clock(std::chrono::time_point<std::chrono::system_clock> now) const -> std::string;

    [[nodiscard]] auto format_indicator_clock(std::chrono::time_point<std::chrono::system_clock> now) const -> std::string;

    [[nodiscard]] auto format(const std::string& format_template, const std::map<std::string, std::string>& replacements) const -> std::string;

    [[nodiscard]] auto format(const std::vector<Module>& modules,
                              const std::string& m_module_separator,
                              bool is_draw_on_empty,
                              const std::map<Module, std::string>& replacements) const -> std::string;

   protected:
    [[nodiscard]] auto format_clock(std::chrono::time_point<std::chrono::system_clock> now, std::string_view format_str) const -> std::string;

    [[nodiscard]] auto get_config() const -> const Config&;

   private:
    const Config& m_config;

    std::map<Module, std::string> m_module_to_string;
    std::map<std::string, Module> m_string_to_module;
};
}  // namespace wall
