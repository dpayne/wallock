#pragma once

#include <deque>
#include <filesystem>
#include <mutex>
#include "mpv/MpvResourceConfig.hpp"

namespace wall {
struct PrimaryDisplayState {
    std::mutex m_guard;
    std::deque<std::filesystem::path> m_lock_files{};
    wall::MpvResourceConfig m_lock_config{};

    std::deque<std::filesystem::path> m_wallpaper_files{};
    wall::MpvResourceConfig m_wallpaper_config{};

    std::string m_primary_name;
};

}  // namespace wall
