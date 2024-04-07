#pragma once

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string_view>

namespace wall {

class Config;

class Log {
   public:
    static auto setup_default_logger(const Config& config) -> void;

    static auto setup_debug_logger(const Config& config) -> void;

    static auto __flush() -> void;  // NOLINT

    static auto set_log_level(std::string_view log_level) -> void;

   private:
    static auto get_log_level(std::string_view log_level) -> spdlog::level::level_enum;
};

#define LOG_TRACE(...) /* NOLINT */                  \
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE) { \
        SPDLOG_TRACE(__VA_ARGS__);                   \
    }

#define LOG_DEBUG(...) /* NOLINT */                  \
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG) { \
        SPDLOG_DEBUG(__VA_ARGS__);                   \
    }

#define LOG_INFO(...) /* NOLINT */                  \
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO) { \
        SPDLOG_INFO(__VA_ARGS__);                   \
    }

#define LOG_WARN(...) /* NOLINT */                  \
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN) { \
        SPDLOG_WARN(__VA_ARGS__);                   \
    }

#define LOG_ERROR(...) /* NOLINT */                  \
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR) { \
        SPDLOG_ERROR(__VA_ARGS__);                   \
    }

#define LOG_FATAL(...) /* NOLINT */                     \
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL) { \
        SPDLOG_CRITICAL(__VA_ARGS__);                   \
        ::wall::Log::__flush();                         \
        exit(1);                                        \
    }
}  // namespace wall
