#include "mpv/MpvScreenshot.hpp"

#include <spdlog/common.h>
#include <array>
#include <csignal>
#include <cstdlib>
#include <map>
#include <optional>
#include <system_error>
#include <thread>

#include "conf/ConfigMacros.hpp"
#include "mpv/MpvResource.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "surface/Surface.hpp"
#include "util/FileUtils.hpp"
#include "util/Log.hpp"
#include "util/StringUtils.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::MpvScreenshot::MpvScreenshot(const Config& config) : m_config(config), m_formatter{get_config()} {}

wall::MpvScreenshot::~MpvScreenshot() { stop(); }

auto wall::MpvScreenshot::get_config() const -> const Config& { return m_config; }

auto wall::MpvScreenshot::set_resource_config(MpvResourceConfig* resource_config) -> void { m_resource_config = resource_config; }

auto wall::MpvScreenshot::stop() -> void {
    if (m_screenshot_data != nullptr) {
        std::lock_guard<std::mutex> lock(m_screenshot_data->m_access_guard);
        if (m_screenshot_data != nullptr) {
            m_screenshot_data->m_mpv_screenshot = nullptr;
            m_screenshot_data = nullptr;
        }
    }
}

auto wall::MpvScreenshot::load_options(MpvResource* mpv_resource) -> void {
    m_screenshot_filename_format = StringUtils::trim(wall_conf_get(get_config(), file, screenshot_filename));
    m_screenshot_format = StringUtils::trim(wall_conf_get(get_config(), file, screenshot_format));

    m_tmp_filename = "screenshot_" + mpv_resource->get_surface()->get_output_name() + "_tmp";
    const auto screenshot_dir = FileUtils::get_expansion_cache(m_resource_config->m_screenshot_directory).value_or("").string();

    if (screenshot_dir.empty()) {
        LOG_FATAL("screenshot directory is empty but screenshot is enabled");
    }

    mpv_resource->send_mpv_cmd("set", "screenshot-template", m_tmp_filename.c_str());
    mpv_resource->send_mpv_cmd("set", "screenshot-dir", screenshot_dir.c_str());
    mpv_resource->send_mpv_cmd("set", "screenshot-format", m_screenshot_format.c_str());
}

auto wall::MpvScreenshot::screenshot([[maybe_unused]] const std::filesystem::path& current_filename, [[maybe_unused]] mpv_handle* mpv) -> void {
    if (!m_resource_config->m_is_screenshot_enabled) {
        return;
    }

    if (m_screenshot_data != nullptr) {
        LOG_DEBUG("screenshot already in progress");
        return;
    }

    auto base_filename = current_filename.filename();
    base_filename.replace_extension("");

    LOG_DEBUG("Taking screenshot of {}", base_filename.string());
    const auto replacements = std::map<std::string, std::string>{{"filename", base_filename.string()}, {"format", m_screenshot_format}};
    const auto screenshot_name_final = m_formatter.format(m_screenshot_filename_format, replacements);
    const auto screenshot_name_tmp = m_tmp_filename + "." + m_screenshot_format;
    const auto screenshot_file_tmp = FileUtils::get_expansion_cache(m_resource_config->m_screenshot_directory).value_or("") / screenshot_name_tmp;
    const auto screenshot_file_final = FileUtils::get_expansion_cache(m_resource_config->m_screenshot_directory).value_or("") / screenshot_name_final;

    // take screenshot in uv worker thread
    m_screenshot_data = new ScreenshotData;
    m_screenshot_data->m_cmd = m_resource_config->m_screenshot_done_cmd;
    m_screenshot_data->m_mpv_screenshot = shared_from_this();
    m_screenshot_data->m_mpv = mpv;
    m_screenshot_data->m_screenshot_delay = m_resource_config->m_screenshot_delay_ms;
    m_screenshot_data->m_screenshot_file = screenshot_file_final;
    m_screenshot_data->m_screenshot_tmp_file = screenshot_file_tmp;
    m_screenshot_data->m_is_screenshot_cache_enabled = m_resource_config->m_is_screenshot_cache_enabled;
    m_screenshot_data->m_is_reload_colors_on_success = m_resource_config->m_is_reload_colors_on_success;

    m_screenshot_thread = std::thread(&MpvScreenshot::take_screenshot, m_screenshot_data);
    m_screenshot_thread.detach();
}

auto wall::MpvScreenshot::take_screenshot(ScreenshotData* data) -> void {
    auto* mpv = data->m_mpv;

    {
        std::lock_guard<std::mutex> lock(data->m_access_guard);
        if (data->m_mpv_screenshot == nullptr) {
            delete data;
            return;
        }
        if (data->m_is_screenshot_cache_enabled && std::filesystem::exists(data->m_screenshot_file)) {
            data->m_mpv_screenshot->m_screenshot_data = nullptr;

            LOG_DEBUG("Using screenshot from cache: {}", data->m_screenshot_file.string());
            const auto is_successful = run_screenshot_callbacks(data->m_screenshot_file, data->m_cmd);
            if (is_successful && data->m_is_reload_colors_on_success) {
                raise(SIGUSR1);
            }

            delete data;
            return;
        }
    }

    std::this_thread::sleep_for(data->m_screenshot_delay);

    {
        std::lock_guard<std::mutex> lock(data->m_access_guard);
        if (data->m_mpv_screenshot == nullptr) {
            // screenshot was cancelled
            delete data;
            return;
        }

        if (std::filesystem::exists(data->m_screenshot_tmp_file)) {
            std::filesystem::remove(data->m_screenshot_tmp_file);
        }
        std::array<const char*, 2> cmd_args = {"screenshot", nullptr};
        mpv_command(mpv, cmd_args.data());

        // clear the screenshot data
        data->m_mpv_screenshot->m_screenshot_data = nullptr;
    }

    // wait for screenshot to be written, the file size will be non-zero
    std::chrono::milliseconds timeout{10 * 1000};
    std::error_code err_code;
    while (std::filesystem::file_size(data->m_screenshot_tmp_file, err_code) == 0 && timeout.count() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        timeout -= std::chrono::milliseconds{1};
    }

    if (std::filesystem::file_size(data->m_screenshot_tmp_file, err_code) == 0) {
        delete data;
        return;
    }

    std::filesystem::rename(data->m_screenshot_tmp_file, data->m_screenshot_file, err_code);

    if (!err_code) {
        const auto is_successful = run_screenshot_callbacks(data->m_screenshot_file, data->m_cmd);
        if (is_successful && data->m_is_reload_colors_on_success) {
            raise(SIGUSR1);
        }
    }

    if (!data->m_is_screenshot_cache_enabled) {
        // give enough time for the callback commands to run before deleting the screenshot
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::filesystem::remove(data->m_screenshot_file);
    }

    delete data;
}

auto wall::MpvScreenshot::replace_filename(const std::string& replace, std::string& subject) -> void {
    static const std::string k_search = "{filename}";
    size_t pos = 0;
    while ((pos = subject.find(k_search, pos)) != std::string::npos) {
        subject.replace(pos, k_search.length(), replace);
        pos += replace.length();
    }
}

auto wall::MpvScreenshot::run_screenshot_callbacks(const std::filesystem::path& screenshot_file, const std::string& cmd) -> bool {
    if (!cmd.empty()) {
        const auto filename = screenshot_file.string();
        // replace all instances of {filename} with the actual filename
        auto final_cmd = cmd;
        replace_filename(filename, final_cmd);
        LOG_DEBUG("Running screenshot done command: {}", final_cmd);
        if (system(final_cmd.c_str()) != 0) {
            LOG_ERROR("screenshot done command failed: {}", final_cmd);
            return false;
        }
    }

    return true;
}
