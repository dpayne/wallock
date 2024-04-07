#include "util/FileUtils.hpp"
#include "util/Log.hpp"

#include <wordexp.h>

namespace {
const std::vector<std::filesystem::path> k_config_dirs = {"$XDG_CONFIG_HOME/wallock/", "$HOME/.config/wallock/", "/etc/wallock/"};
const std::vector<std::filesystem::path> k_share_dirs = {"$XDG_DATA_HOME/wallock/", "$HOME/.local/share/wallock/", "/tmp/wallock/"};
const std::vector<std::filesystem::path> k_cache_dirs = {"$XDG_CACHE_HOME/wallock/", "$HOME/.cache/wallock/", "/tmp/wallock/"};
}  // namespace

auto wall::FileUtils::get_default_data_dir() -> std::filesystem::path {
    auto data_dir = std::filesystem::path{};

    auto* xdg_data_home = std::getenv("XDG_DATA_HOME");
    if (xdg_data_home != nullptr) {
        return std::filesystem::path{xdg_data_home} / "wallock";
    }

    auto* home = std::getenv("HOME");
    if (home != nullptr) {
        return std::filesystem::path{home} / ".local/share/wallock";
    }
    return "/tmp/wallock";
}

auto wall::FileUtils::get_default_runtime_dir() -> std::filesystem::path {
    auto runtime_dir = std::filesystem::path{};

    auto* xdg_runtime_home = std::getenv("XDG_RUNTIME_DIR");
    if (xdg_runtime_home != nullptr) {
        runtime_dir = std::filesystem::path{xdg_runtime_home} / "wallock";
    } else {
        runtime_dir = "/tmp/wallock";
    }

    return runtime_dir;
}

auto wall::FileUtils::get_expansion_config(const std::filesystem::path& file) -> std::optional<std::filesystem::path> {
    return get_expansion(file, k_config_dirs, true);
}

auto wall::FileUtils::get_expansion_data(const std::filesystem::path& file) -> std::optional<std::filesystem::path> {
    return get_expansion(file, k_share_dirs);
}

auto wall::FileUtils::get_expansion_cache(const std::filesystem::path& file) -> std::optional<std::filesystem::path> {
    return get_expansion(file, k_cache_dirs);
}

auto wall::FileUtils::get_expansion(const std::filesystem::path& file, const std::vector<std::filesystem::path>& root_dirs, bool is_exists)
    -> std::optional<std::filesystem::path> {
    if (file.is_absolute()) {
        return file;
    }

    auto result = expand_path(file);
    if (result && std::filesystem::exists(*result)) {
        return *result;
    }

    for (const auto& dir : root_dirs) {
        auto path = dir / file;
        result = expand_path(path);
        if (result) {
            if (is_exists && std::filesystem::exists(*result)) {
                return *result;
            }

            if (!is_exists) {
                return *result;
            }
        }

        LOG_TRACE("Failed to expand path: {} result {}", path.string(), result.has_value() ? result.value().string() : "nullopt");
    }

    return {};
}

auto wall::FileUtils::expand_path(const std::filesystem::path& path) -> std::optional<std::filesystem::path> {
    std::unordered_map<std::string, std::string> replacements;
    auto* env = std::getenv("HOME");
    if (env != nullptr) {
        replacements["$HOME"] = env;
        replacements["~"] = env;
    }

    env = std::getenv("XDG_CONFIG_HOME");
    if (env != nullptr) {
        replacements["$XDG_CONFIG_HOME"] = env;
    }

    env = std::getenv("XDG_DATA_HOME");
    if (env != nullptr) {
        replacements["$XDG_DATA_HOME"] = env;
    }

    env = std::getenv("XDG_RUNTIME_DIR");
    if (env != nullptr) {
        replacements["$XDG_RUNTIME_DIR"] = env;
    }

    // replace all environment variables in the path
    std::filesystem::path result = path;
    for (const auto& [key, value] : replacements) {
        if (result.string().starts_with(key)) {
            result = result.string().replace(0, key.size(), value);
        }
    }

    if (result.string().find('$') != std::string::npos) {
        return std::nullopt;
    }

    return result;
}

auto wall::FileUtils::get_all_files(const std::filesystem::path& dir, const std::set<std::string>& valid_extensions)
    -> std::deque<std::filesystem::path> {
    std::deque<std::filesystem::path> files;
    const auto expanded_path_opt = FileUtils::expand_path(dir);
    if (!expanded_path_opt) {
        LOG_ERROR("Failed to expand path: {}", dir.string());
        return files;
    }
    const auto& expanded_path = *expanded_path_opt;

    if (!std::filesystem::exists(expanded_path)) {
        LOG_ERROR("Directory does not exist: {}", expanded_path.string());
        return files;
    }

    if (!std::filesystem::is_directory(expanded_path)) {
        files.push_back(expanded_path);
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(expanded_path)) {
        if (entry.is_regular_file()) {
            if (valid_extensions.empty() || valid_extensions.contains(entry.path().extension())) {
                files.push_back(entry.path());
            }
        }
    }

    return files;
}
