#pragma once

#include "display/PrimaryDisplayState.hpp"
#include "mpv/MpvResource.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "util/Loop.hpp"

#include <deque>
#include <functional>
#include <string>

namespace wall {
class MpvResource;

class MpvFileLoader {
   public:
    MpvFileLoader(const Config& config,
                  Loop* loop,
                  MpvResourceConfig* resource_config,
                  PrimaryDisplayState* primary_state,
                  std::function<void(std::string)> on_load_file);

    virtual ~MpvFileLoader();

    auto stop() -> void;

    auto load_options() -> void;

    auto set_resource_config(MpvResourceConfig* resource_config) -> void;

    /**
     * @brief Sets up a timer to load the next file after a certain duration.
     *
     * This function sets up a timer to automatically load the next file after a specified duration.
     * If there is only one file found, the timer is not set up.
     * The file duration is adjusted based on the resource configuration to prevent black frames.
     *
     * @param file_duration The duration of the current file.
     */
    auto setup_load_next_file_timer(double file_duration) -> void;

    /**
     * @brief Loads the next file for playback using MPV.
     *
     * This function loads the next file from the list of files to be played using MPV.
     * If there are no files available, it logs a debug message and returns.
     *
     * @return void
     */
    auto load_next_file() -> void;

    [[nodiscard]] auto get_current_file() const -> const std::filesystem::path&;

    static auto calculate_timer_delay(double file_duration, const MpvResourceConfig& resource_config) -> std::chrono::seconds;

   protected:
    auto assign_global_order(const std::deque<std::filesystem::path>& files) const -> void;

    auto get_files(const MpvResourceConfig& resource_config) -> std::deque<std::filesystem::path>;

    [[nodiscard]] auto get_loop() const -> Loop*;

    [[nodiscard]] auto get_config() const -> const Config&;

   private:
    const Config& m_config;

    Loop* m_loop{};

    MpvResourceConfig* m_resource_config{};

    PrimaryDisplayState* m_primary_state{};

    std::function<void(std::string)> m_on_load_file{};

    std::deque<std::filesystem::path> m_files;

    uint64_t m_load_file_counter{};

    loop::Timer* m_load_next_file_timer{};

    std::filesystem::path m_current_file;
};
}  // namespace wall
