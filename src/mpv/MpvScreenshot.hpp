#pragma once

#include <string>
#include <thread>
#include "mpv/MpvResource.hpp"
#include "util/Formatter.hpp"

namespace wall {
class MpvResource;
class MpvScreenshot : public std::enable_shared_from_this<MpvScreenshot> {
   public:
    MpvScreenshot(const Config& config);

    ~MpvScreenshot();

    auto stop() -> void;

    auto load_options(MpvResource* mpv_resource) -> void;

    auto screenshot(const std::filesystem::path& current_filename, mpv_handle* mpv) -> void;

    auto set_resource_config(MpvResourceConfig* resource_config) -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    static auto replace_filename(const std::string& replace, std::string& subject) -> void;

    static auto run_screenshot_callbacks(const std::filesystem::path& screenshot_file, const std::string& cmd) -> bool;

   private:
    struct ScreenshotData {
        mpv_handle* m_mpv{};
        std::string m_cmd;
        bool m_is_screenshot_cache_enabled{};
        bool m_is_reload_colors_on_success{false};
        std::filesystem::path m_screenshot_file;
        std::filesystem::path m_screenshot_tmp_file;
        std::chrono::milliseconds m_screenshot_delay;
        std::mutex m_access_guard{};
        std::shared_ptr<MpvScreenshot> m_mpv_screenshot{};
    };

    static auto take_screenshot(ScreenshotData* data) -> void;

    const Config& m_config;

    std::string m_tmp_filename;

    std::thread m_screenshot_thread;

    ScreenshotData* m_screenshot_data{};

    MpvResourceConfig* m_resource_config{};

    std::string m_screenshot_filename_format;

    std::string m_screenshot_format;

    Formatter m_formatter;
};
}  // namespace wall
