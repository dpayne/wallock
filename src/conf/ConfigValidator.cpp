#include "conf/ConfigValidator.hpp"

#include <filesystem>
#include "conf/ConfigMacros.hpp"
#include "util/CommandProcessor.hpp"
#include "util/FileUtils.hpp"

auto wall::ConfigValidator::validate(const Config& config) -> bool {
    return is_resource_paths_valid(config) && is_pam_file_installed(config) && !is_already_running(config);
}

auto wall::ConfigValidator::is_resource_paths_valid(const Config& config) -> bool {
    const auto file_path_str = wall_conf_get(config, file, path);
    const auto lock_path_str = wall_conf_get(config, lock, path);
    const auto wallpaper_path_str = wall_conf_get(config, wallpaper, path);

    if (file_path_str.empty() && lock_path_str.empty() && wallpaper_path_str.empty()) {
        LOG_FATAL("No resource paths provided, please set either file_path or, lock_path and wallpaper_path in the config file.");
        return false;
    }

    const auto is_lock_path_valid = lock_path_str.empty() || is_resource_path_valid(lock_path_str);
    if (!is_lock_path_valid) {
        LOG_FATAL("Invalid lock path provided, please provide a valid lock path.");
        return false;
    }

    const auto is_wallpaper_path_valid = wallpaper_path_str.empty() || is_resource_path_valid(wallpaper_path_str);
    if (!is_wallpaper_path_valid) {
        LOG_FATAL("Invalid wallpaper path provided, please provide a valid wallpaper path.");
        return false;
    }

    // file path only matters if lock path or wallpaper path is not provided
    if (wallpaper_path_str.empty() || lock_path_str.empty()) {
        const auto is_file_path_valid = file_path_str.empty() || is_resource_path_valid(file_path_str);
        if (!is_file_path_valid) {
            LOG_FATAL("Invalid file path provided, please provide a valid file path.");
            return false;
        }
    }

    return true;
}

auto wall::ConfigValidator::is_resource_path_valid(std::string_view path_str) -> bool {
    const auto file_path_opt = FileUtils::expand_path(path_str);
    if (!file_path_opt.has_value()) {
        LOG_ERROR("Invalid path provided: {}", path_str);
        return false;
    }

    if (FileUtils::get_all_files(file_path_opt.value()).empty()) {
        LOG_ERROR("No files found for path: {}", file_path_opt.value().string());
        return false;
    }

    return true;
}

auto wall::ConfigValidator::is_pam_file_installed(const Config& config) -> bool {
    const auto pam_file_path = std::filesystem::path{"/etc/pam.d/wallock"};
    if (!std::filesystem::exists(pam_file_path)) {
        LOG_ERROR("Pam file not found at path: {}", pam_file_path.string());
        return false;
    }

    return true;
}

auto wall::ConfigValidator::is_already_running(const Config& config) -> bool {
    const auto is_ignore_running = wall_conf_get(config, command, ignore_is_running);
    if (is_ignore_running) {
        return false;
    }
    const auto cmd = wall_conf_get(config, command, name);

    if (cmd.empty() && CommandProcessor::is_running(config)) {
        LOG_ERROR("Already running, if this is a mistake, please remove the file: {}", CommandProcessor::get_socket_filename(config).string());
        return true;
    }

    return false;
}
