#include "mpv/MpvResourceConfig.hpp"

#include <spdlog/common.h>
#include <cstdint>
#include <sstream>

#include "util/Log.hpp"
#include "util/StringUtils.hpp"

auto wall::MpvResourceConfig::build_config(const Config& config, ResourceMode mode) -> MpvResourceConfig {
    const auto* config_prefix = mode == ResourceMode::Wallpaper ? "wallpaper" : "lock";
    return build_config(config, config_prefix, mode);
}

auto wall::MpvResourceConfig::build_config(const Config& config, const char* config_prefix, ResourceMode mode) -> MpvResourceConfig {
    MpvResourceConfig resource_config;

    resource_config.m_mode = mode;
    resource_config.m_path = StringUtils::trim(get_config_with_fallback<std::string>(config, config_prefix, "path"));
    std::string extensions = StringUtils::trim(std::string{get_config_with_fallback<std::string>(config, config_prefix, "extensions")});
    resource_config.m_is_mute = get_config_with_fallback<bool>(config, config_prefix, "mute");
    const auto fit_mode = StringUtils::trim(get_config_with_fallback<std::string>(config, config_prefix, "fit"));
    const auto order = StringUtils::trim(get_config_with_fallback<std::string>(config, config_prefix, "sort_order"));
    resource_config.m_is_keep_same_order = get_config_with_fallback<bool>(config, config_prefix, "keep_same_order");
    resource_config.m_image_change_interval_secs =
        std::chrono::seconds(get_config_with_fallback<uint64_t>(config, config_prefix, "image_change_interval_secs"));
    resource_config.m_video_preload_secs = std::chrono::seconds(get_config_with_fallback<uint64_t>(config, config_prefix, "video_preload_secs"));
    resource_config.m_video_max_change_interval_secs =
        std::chrono::seconds(get_config_with_fallback<uint64_t>(config, config_prefix, "video_max_change_interval_secs"));
    resource_config.m_is_screenshot_enabled = get_config_with_fallback<bool>(config, config_prefix, "screenshot_enabled");
    resource_config.m_is_screenshot_cache_enabled = get_config_with_fallback<bool>(config, config_prefix, "screenshot_cache_enabled");
    resource_config.m_screenshot_directory = StringUtils::trim(get_config_with_fallback<std::string>(config, config_prefix, "screenshot_directory"));
    resource_config.m_screenshot_delay_ms =
        std::chrono::milliseconds(get_config_with_fallback<uint64_t>(config, config_prefix, "screenshot_delay_ms"));
    resource_config.m_screenshot_done_cmd = StringUtils::trim(get_config_with_fallback<std::string>(config, config_prefix, "screenshot_done_cmd"));
    resource_config.m_is_reload_colors_on_success = get_config_with_fallback<bool>(config, config_prefix, "screenshot_reload_on_done");

    std::istringstream iss{extensions};
    std::string extension;
    while (std::getline(iss, extension, ',')) {
        resource_config.m_extensions.insert("." + extension);
    }

    if (order == "random") {
        resource_config.m_order = Order::Random;
    } else {
        resource_config.m_order = Order::Alpha;
    }

    if (fit_mode == "fit") {
        resource_config.m_fit_mode = FitMode::Fit;
    } else if (fit_mode == "fill") {
        resource_config.m_fit_mode = FitMode::Fill;
    }

    LOG_DEBUG("MpvResourceConfig: {}", resource_config.to_string());

    return resource_config;
}

auto wall::MpvResourceConfig::is_resource_modes_compatible(const Config& config, ResourceMode mode_1, ResourceMode mode_2) -> bool {
    const auto config_1 = build_config(config, mode_1);
    const auto config_2 = build_config(config, mode_2);

    // if the path and extensions are the same, the resources are compatible
    return config_1.m_path == config_2.m_path && config_1.m_extensions == config_2.m_extensions;
}

auto wall::MpvResourceConfig::operator==(const MpvResourceConfig& other) const -> bool {
    return m_path == other.m_path && m_extensions == other.m_extensions && m_is_mute == other.m_is_mute &&
           m_is_screenshot_enabled == other.m_is_screenshot_enabled && m_is_screenshot_cache_enabled == other.m_is_screenshot_cache_enabled &&
           m_screenshot_directory == other.m_screenshot_directory && m_screenshot_delay_ms == other.m_screenshot_delay_ms &&
           m_image_change_interval_secs == other.m_image_change_interval_secs &&
           m_video_max_change_interval_secs == other.m_video_max_change_interval_secs && m_video_preload_secs == other.m_video_preload_secs &&
           m_fit_mode == other.m_fit_mode && m_order == other.m_order;
}

auto wall::MpvResourceConfig::to_string() const -> std::string {
    auto bool_to_string = [](bool value) -> std::string { return value ? "true" : "false"; };

    std::string result;
    result += "path: " + m_path + "\n";
    result += "extensions: ";
    for (const auto& extension : m_extensions) {
        result += extension + ", ";
    }
    result += "\n";
    result += "is_mute: " + bool_to_string(m_is_mute) + "\n";
    result += "is_screenshot_enabled: " + bool_to_string(m_is_screenshot_enabled) + "\n";
    result += "is_screenshot_cache_enabled: " + bool_to_string(m_is_screenshot_cache_enabled) + "\n";
    result += "screenshot_directory: " + m_screenshot_directory + "\n";
    result += "screenshot_delay_ms: " + std::to_string(m_screenshot_delay_ms.count()) + "\n";
    result += "image_change_interval_secs: " + std::to_string(m_image_change_interval_secs.count()) + "\n";
    result += "video_max_change_interval_secs: " + std::to_string(m_video_max_change_interval_secs.count()) + "\n";
    result += "video_preload_secs: " + std::to_string(m_video_preload_secs.count()) + "\n";
    result += "fit_mode: " + std::to_string(static_cast<int>(m_fit_mode)) + "\n";
    result += "order: " + std::to_string(static_cast<int>(m_order)) + "\n";
    return result;
}
