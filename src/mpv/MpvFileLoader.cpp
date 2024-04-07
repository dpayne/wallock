#include "mpv/MpvFileLoader.hpp"

#include <spdlog/common.h>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <random>
#include <stdexcept>
#include <utility>

#include "display/PrimaryDisplayState.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "util/FileUtils.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::MpvFileLoader::MpvFileLoader(const Config& config,
                                   Loop* loop,
                                   MpvResourceConfig* resource_config,
                                   PrimaryDisplayState* primary_state,
                                   std::filesystem::path current_file,
                                   double last_seek_position,
                                   std::function<void(std::string, double)> on_load_file)
    : m_config{config},
      m_last_file{std::move(current_file)},
      m_last_seek_position{last_seek_position},
      m_loop{loop},
      m_resource_config{resource_config},
      m_primary_state{primary_state},
      m_on_load_file(std::move(on_load_file)) {}

wall::MpvFileLoader::~MpvFileLoader() { stop(); }

auto wall::MpvFileLoader::stop() -> void {
    if (m_load_next_file_timer != nullptr) {
        m_load_next_file_timer->close();
    }
}

auto wall::MpvFileLoader::set_resource_config(MpvResourceConfig* resource_config) -> void { m_resource_config = resource_config; }

auto wall::MpvFileLoader::get_current_file() const -> const std::filesystem::path& { return m_current_file; }

auto wall::MpvFileLoader::get_loop() const -> Loop* { return m_loop; }

auto wall::MpvFileLoader::load_options() -> void {
    m_files = get_files(*m_resource_config);
    if (m_files.empty()) {
        LOG_ERROR("No files found in {}", m_resource_config->m_path);
        throw std::runtime_error("No files found");
    }
    assign_global_order(m_files);
}

auto wall::MpvFileLoader::assign_global_order(const std::deque<std::filesystem::path>& files) const -> void {
    if (m_resource_config->m_is_keep_same_order) {
        std::lock_guard<std::mutex> lock(m_primary_state->m_guard);
        if (m_resource_config->m_mode == ResourceMode::Wallpaper) {
            m_primary_state->m_wallpaper_files = files;
            m_primary_state->m_wallpaper_config = *m_resource_config;
        } else {
            m_primary_state->m_lock_files = files;
            m_primary_state->m_lock_config = *m_resource_config;
        }
    }
}

auto wall::MpvFileLoader::get_files(const MpvResourceConfig& resource_config) -> std::deque<std::filesystem::path> {
    if (resource_config.m_is_keep_same_order) {
        auto* primary_state = m_primary_state;
        std::lock_guard<std::mutex> lock(primary_state->m_guard);
        if (resource_config.m_mode == ResourceMode::Wallpaper) {
            if (!primary_state->m_wallpaper_files.empty() && primary_state->m_wallpaper_config == resource_config) {
                return primary_state->m_wallpaper_files;
            }
        } else {
            if (!primary_state->m_lock_files.empty() && primary_state->m_lock_config == resource_config) {
                return primary_state->m_lock_files;
            }
        }
    }

    auto files = FileUtils::get_all_files(resource_config.m_path, resource_config.m_extensions);

    // random order
    switch (resource_config.m_order) {
        case Order::Random: {
            std::shuffle(files.begin(), files.end(), std::mt19937(std::random_device()()));
            break;
        }
        case Order::Alpha:
        default: {
            std::sort(files.begin(), files.end());
            break;
        }
    }

    return files;
}

auto wall::MpvFileLoader::calculate_timer_delay(double file_duration, const MpvResourceConfig& resource_config) -> std::chrono::seconds {
    if (file_duration == 0.0) {
        file_duration = resource_config.m_image_change_interval_secs.count();
    } else if (resource_config.m_video_max_change_interval_secs.count() != 0 &&
               file_duration > resource_config.m_video_max_change_interval_secs.count()) {
        file_duration = resource_config.m_video_max_change_interval_secs.count();
    }

    // Calculate the timer delay to prevent black frames
    auto timer_delay = std::chrono::seconds{static_cast<uint64_t>(file_duration)};
    if (timer_delay > resource_config.m_video_preload_secs) {
        timer_delay -= resource_config.m_video_preload_secs;
    }

    return timer_delay;
}

auto wall::MpvFileLoader::setup_load_next_file_timer([[maybe_unused]] double file_duration) -> void {
    if (m_files.size() <= 1) {
        LOG_DEBUG("Only one file found, not setting up load next file timer");
        return;
    }

    if (m_load_next_file_timer != nullptr) {
        // Close the existing timer handle
        m_load_next_file_timer->close();
        m_load_next_file_timer = nullptr;
    }

    // Adjust the file duration based on resource configuration
    const auto timer_delay = calculate_timer_delay(file_duration, *m_resource_config);

    // Create a new timer handle
    m_load_next_file_timer = get_loop()->add_timer(timer_delay, std::chrono::milliseconds{0}, [this](loop::Timer*) { load_next_file(); });
}

auto wall::MpvFileLoader::load_next_file() -> void {
    if (!m_last_file.empty()) {
        m_on_load_file(m_last_file, m_last_seek_position);
        m_load_file_counter++;

        m_current_file = m_last_file;

        m_last_file = std::filesystem::path{};
        m_last_seek_position = 0.0;

        return;
    }

    // Check if there are files to load
    if (m_files.empty()) {
        LOG_DEBUG("No files to load");
        return;
    }

    // Handle case where file order needs to be maintained
    if (m_resource_config->m_is_keep_same_order) {
        std::lock_guard<std::mutex> lock(m_primary_state->m_guard);
        if (m_resource_config->m_mode == ResourceMode::Wallpaper) {
            m_files = m_primary_state->m_wallpaper_files;
        } else {
            m_files = m_primary_state->m_lock_files;
        }
    }

    auto file = m_files[m_load_file_counter];
    LOG_INFO("Loading file: {}", file.string());

    // Send the MPV command to load the file
    m_on_load_file(file, 0.0);

    m_load_file_counter++;

    // Re-shuffle files at the end of a rotation if order is random
    if (m_load_file_counter >= m_files.size() && m_resource_config->m_order == Order::Random) {
        m_load_file_counter = 0;
        std::shuffle(m_files.begin(), m_files.end(), std::mt19937(std::random_device()()));

        if (m_files.front() == file && m_files.size() > 1) {
            // Ensure we do not repeat the same file twice in a row
            std::rotate(m_files.begin(), m_files.begin() + 1, m_files.end());
        }

        assign_global_order(m_files);
    }

    // add the current file back to the list
    m_files.push_back(file);
    m_current_file = file;
}
