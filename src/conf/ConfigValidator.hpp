#pragma once

#include "conf/Config.hpp"

namespace wall {
class ConfigValidator {
   public:
    static auto validate(const Config& config) -> bool;

   private:
    static auto is_resource_paths_valid(const Config& config) -> bool;

    static auto is_resource_path_valid(std::string_view path_str) -> bool;

    static auto is_pam_file_installed() -> bool;

    static auto is_already_running(const Config& config) -> bool;
};
}  // namespace wall
