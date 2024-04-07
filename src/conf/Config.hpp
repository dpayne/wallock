#pragma once

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#include <spdlog/common.h>
#include <cxxopts.hpp>
#pragma GCC diagnostic pop
#include <set>

#include "conf/ConfigDefaultSettings.hpp"
#include "util/Log.hpp"

namespace wall {

using OptionsMap = std::unordered_map<std::string, conf::SettingsVariantType>;

class Config {
   public:
    Config(int argc, const char** argv);

    static auto get_command_line_options() -> cxxopts::Options;

    static auto get_default_config() -> Config;

    auto reload_options() -> void;

    static auto is_exit_immediately_option(const char* arg) -> bool;

    auto is_set_from_config_file(const std::string& key) const -> bool { return m_options_set_from_config.contains(key); }

    template <typename ValueType>
    auto get_with_fallback(std::string_view key, std::string_view fallback, ValueType default_value) const {
        // if the key is not directly set from the config, but the fallback is, then use the fallback
        if (!is_set_from_config_file(std::string{key}) && is_set_from_config_file(std::string{fallback})) {
            // we know the fallback is set from the config so no need to use a default
            return get<ValueType>(fallback);
        }

        // otherwise, use the value from the config or the default
        return get<ValueType>(key, default_value);
    }

    template <typename ValueType>
    auto get(std::string_view key, ValueType default_value = ValueType{}) const {
        auto result = m_options.find(std::string{key});

        if constexpr (std::is_same_v<ValueType, bool>) {
            if (result == m_options.end()) {
                return default_value;
            }
            return std::get<bool>(result->second);
        } else if constexpr (std::is_floating_point_v<ValueType>) {  // note, should be after bool
            if (result == m_options.end()) {
                return default_value;
            }
            return static_cast<ValueType>(std::get<double>(result->second));
        } else if constexpr (std::is_integral_v<ValueType> && std::is_unsigned_v<ValueType>) {  // note, should be after bool
            if (result == m_options.end()) {
                return default_value;
            }
            return static_cast<ValueType>(std::get<unsigned long>(result->second));
        } else if constexpr (std::is_integral_v<ValueType>) {  // note, should be after bool
            if (result == m_options.end()) {
                return default_value;
            }
            return static_cast<ValueType>(std::get<long>(result->second));
        } else if constexpr (std::is_convertible_v<ValueType, std::string>) {  // note should be last
            if (result == m_options.end()) {
                return std::string_view{default_value};
            }
            return std::string_view{std::get<std::string>(result->second).c_str(), std::get<std::string>(result->second).size()};
        } else {
            throw std::runtime_error("Unsupported type in Config lookup for " + std::string(key));
        }
    }

    [[nodiscard]] auto is_debug() const -> bool {
        auto find_result = m_options.find(conf::k_debug_mode);
        if (find_result == m_options.end()) {
            return conf::k_default_debug_mode;
        }
        return std::get<bool>(find_result->second);
    }

#ifdef DEBUG
    template <typename ValueType>
    auto set(std::string_view key, ValueType value) {
        m_options[std::string{key}] = value;
    }
#endif

    auto get_color_name(const std::string& value) const -> std::string;

   private:
    Config();

    static auto read_config(std::istream& config_file, OptionsMap& config_options, std::set<std::string>& options_set_from_config) -> void;

    static auto open_file_for_reading(std::string_view filename) -> std::optional<std::ifstream>;

    auto load_options() -> void;

    auto load_color_scheme_file() -> void;

    auto replace_color_scheme_colors() -> void;

    static auto add_general_options() -> OptionsMap;

    static auto read_config_file(OptionsMap& config_options, std::set<std::string>& options_set_from_config) -> void;

    static auto process_cmd_line_args(const cxxopts::Options& options, const cxxopts::ParseResult& result, OptionsMap& config_options) -> void;

    static auto process_config_pair(const std::string& key,
                                    const std::string& value,
                                    OptionsMap& config_options,
                                    std::set<std::string>& options_set_from_config) -> void;

    template <typename ValueType>
    static auto add_option(std::string name, ValueType default_value, OptionsMap& options) -> void {  // NOLINT *-performance-unnecessary-value-param
        bool is_added = false;
        if constexpr (std::is_same_v<ValueType, bool>) {
            auto result = options.emplace(name, default_value);
            is_added = result.second;
        } else if constexpr (std::is_floating_point_v<ValueType>) {  // note, should be after bool
            auto result = options.emplace(name, static_cast<double>(default_value));
            is_added = result.second;
        } else if constexpr (std::is_integral_v<ValueType>) {  // note, should be after bool
            auto result = options.emplace(name, static_cast<long>(default_value));
            is_added = result.second;
        } else if constexpr (std::is_convertible_v<ValueType, std::string>) {  // note should be last
            auto result = options.emplace(name, std::string(default_value));
            is_added = result.second;
        } else {
            LOG_FATAL("Unsupported type {}", name);
        }

        if (!is_added) {
            LOG_WARN("Option {} already exists, overriding", name);
        }
    }

    OptionsMap m_options;
    OptionsMap m_cmd_line_options;
    std::set<std::string> m_options_set_from_config;
};
}  // namespace wall
