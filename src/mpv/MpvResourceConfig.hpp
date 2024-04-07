#pragma once

#include <chrono>
#include <set>
#include <string>

#include "conf/Config.hpp"
#include "conf/ConfigDefaultSettings.hpp"

namespace wall {

enum class ResourceMode { None, Wallpaper, Lock };

enum class FitMode { None, Fill, Fit };

enum class Order { None, Random, Alpha };

struct MpvResourceConfig {
    static auto build_config(const Config& config, ResourceMode mode) -> MpvResourceConfig;

    static auto is_resource_modes_compatible(const Config& config, ResourceMode mode_1, ResourceMode mode_2) -> bool;

    template <typename ValueType>
    static auto get_config_with_fallback(const Config& config, const std::string& prefix, const char* suffix) {
        // this allows overriding the config with a more specific one
        // e.g. file.path will apply to both wallpaper and lock screen, where wallpaper.path will be only the wallpaper settings
        const auto config_1 = prefix + "_" + suffix;
        const auto config_2 = std::string{"file_"} + suffix;
        if (config.is_set_from_config_file(config_1)) {
            return config.get<ValueType>(config_1);
        }

        return config.get<ValueType>(config_2);
    }

    [[nodiscard]] auto to_string() const -> std::string;

    ResourceMode m_mode{ResourceMode::None};
    std::string m_path;
    std::set<std::string> m_extensions;
    bool m_is_mute{conf::k_default_file_mute};
    bool m_is_screenshot_enabled{conf::k_default_file_screenshot_enabled};
    bool m_is_screenshot_cache_enabled{conf::k_default_file_screenshot_cache_enabled};
    bool m_is_reload_colors_on_success{conf::k_default_file_screenshot_reload_on_done};
    std::string m_screenshot_directory{conf::k_default_file_screenshot_directory};
    std::chrono::milliseconds m_screenshot_delay_ms{std::chrono::milliseconds(conf::k_default_file_screenshot_delay_ms)};
    std::string m_screenshot_done_cmd{conf::k_default_file_screenshot_done_cmd};
    std::chrono::seconds m_image_change_interval_secs{std::chrono::seconds(conf::k_default_file_image_change_interval_secs)};
    std::chrono::seconds m_video_max_change_interval_secs{std::chrono::seconds(conf::k_default_file_video_max_change_interval_secs)};
    std::chrono::seconds m_video_preload_secs{std::chrono::seconds(conf::k_default_file_video_preload_secs)};
    FitMode m_fit_mode{FitMode::Fill};
    Order m_order{Order::None};
    bool m_is_keep_same_order{conf::k_default_file_keep_same_order};

    [[nodiscard]] auto operator==(const MpvResourceConfig& other) const -> bool;

   private:
    static auto build_config(const Config& config, const char* config_prefix, ResourceMode mode) -> MpvResourceConfig;
};

}  // namespace wall
