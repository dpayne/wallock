#pragma once

#include <deque>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace wall {
class FileUtils {
   public:
    static auto expand_path(const std::filesystem::path& path) -> std::optional<std::filesystem::path>;

    static auto get_expansion_config(const std::filesystem::path& file) -> std::optional<std::filesystem::path>;

    static auto get_expansion_data(const std::filesystem::path& file) -> std::optional<std::filesystem::path>;

    static auto get_expansion_cache(const std::filesystem::path& file) -> std::optional<std::filesystem::path>;

    static auto get_default_data_dir() -> std::filesystem::path;

    static auto get_default_runtime_dir() -> std::filesystem::path;

    static auto get_all_files(const std::filesystem::path& dir,
                              const std::set<std::string>& valid_extensions = {}) -> std::deque<std::filesystem::path>;

   private:
    static auto get_expansion(const std::filesystem::path& file,
                              const std::vector<std::filesystem::path>& root_dirs,
                              bool is_exists = false) -> std::optional<std::filesystem::path>;
};
}  // namespace wall
