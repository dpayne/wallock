#include "util/Log.hpp"
#include "conf/Config.hpp"
#include "util/FileUtils.hpp"
#include "util/StringUtils.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace {
std::shared_ptr<spdlog::sinks::sink> g_default_sink = nullptr;                  // NOLINT
std::shared_ptr<spdlog::logger> g_default_logger = nullptr;                     // NOLINT
std::shared_ptr<spdlog::details::thread_pool> g_default_thread_pool = nullptr;  // NOLINT
constexpr std::string_view k_service_name = "wallock";
}  // namespace

auto wall::Log::setup_default_logger([[maybe_unused]] const Config& config) -> void {
    auto log_file_base = StringUtils::trim(std::filesystem::path{wall_conf_get(config, log, file)});
    auto log_file = FileUtils::get_expansion_data(log_file_base).value_or(FileUtils::get_default_data_dir() / log_file_base);

    // make root dir
    std::filesystem::create_directories(log_file.parent_path());

    const auto log_line_count = wall_conf_get(config, log, line_count);
    const auto log_threads = wall_conf_get(config, log, threads);
    const auto log_file_size = wall_conf_get(config, log, file_size);
    const auto log_level = StringUtils::trim(wall_conf_get(config, log, level));

    g_default_thread_pool = std::make_shared<spdlog::details::thread_pool>(log_line_count, log_threads);

    g_default_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(std::string{log_file}, log_file_size, 1);
    g_default_logger = std::make_shared<spdlog::async_logger>(std::string{k_service_name}, g_default_sink, g_default_thread_pool,
                                                              spdlog::async_overflow_policy::overrun_oldest);
    g_default_logger->flush_on(spdlog::level::debug);

    auto default_log_level = get_log_level(log_level);

    g_default_logger->set_level(default_log_level);

    spdlog::set_level(default_log_level);
    spdlog::set_default_logger(g_default_logger);
}

auto wall::Log::get_log_level(std::string_view log_level) -> spdlog::level::level_enum {
    if (log_level == "trace") {
        return spdlog::level::trace;
    }

    if (log_level == "debug") {
        return spdlog::level::debug;
    }

    if (log_level == "info") {
        return spdlog::level::info;
    }

    if (log_level == "warn") {
        return spdlog::level::warn;
    }

    if (log_level == "error") {
        return spdlog::level::err;
    }

    if (log_level == "fatal") {
        return spdlog::level::critical;
    }

    if (log_level == "off") {
        return spdlog::level::off;
    }

    return spdlog::level::off;
}

auto wall::Log::set_log_level(std::string_view log_level) -> void {
    if (g_default_logger != nullptr) {
        g_default_logger->set_level(get_log_level(log_level));
    }
}

auto wall::Log::setup_debug_logger(const Config& config) -> void {
    auto log_level = StringUtils::trim(wall_conf_get(config, log, level));
    if (config.is_debug() && log_level != "trace") {
        log_level = "debug";
    }

    auto default_log_level = get_log_level(log_level);

    g_default_logger = spdlog::stdout_color_mt(std::string{k_service_name});
    g_default_logger->set_level(default_log_level);
    g_default_logger->flush_on(spdlog::level::debug);

    spdlog::set_default_logger(g_default_logger);
    spdlog::set_level(default_log_level);
}

auto wall::Log::__flush() -> void {
    if (g_default_logger != nullptr) {
        g_default_logger->flush();
    }
}
