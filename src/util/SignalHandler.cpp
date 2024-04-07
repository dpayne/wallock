#include "util/SignalHandler.hpp"
#include <csignal>

#include "util/CommandProcessor.hpp"
#include "util/Log.hpp"

namespace {
const wall::Config* g_config = nullptr;
}

wall::SignalHandler::SignalHandler(const Config* config) {
    if (g_config == nullptr) {
        g_config = config;
    } else {
        LOG_WARN("SignalHandler already initialized");
    }

    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);
    signal(SIGQUIT, shutdown_handler);
    signal(SIGUSR1, refresh_handler);
    signal(SIGUSR2, reload_handler);
}

wall::SignalHandler::~SignalHandler() {
    g_config = nullptr;
    stop_listening();
}

auto wall::SignalHandler::refresh_handler([[maybe_unused]] int signal) -> void {
    LOG_DEBUG("Received signal to reload colors {}", signal);
    if (g_config != nullptr) {
        CommandProcessor::send(*g_config, commands::k_reload);
    }
}

auto wall::SignalHandler::reload_handler([[maybe_unused]] int signal) -> void {
    LOG_DEBUG("Received signal to reload {}", signal);
    if (g_config != nullptr) {
        CommandProcessor::send(*g_config, commands::k_full_reload);
    }
}

auto wall::SignalHandler::shutdown_handler([[maybe_unused]] int signal) -> void {
    LOG_DEBUG("Received signal to shutdown {}", signal);
    stop_listening();
    if (g_config != nullptr) {
        CommandProcessor::send(*g_config, commands::k_stop);
    }
    g_config = nullptr;
}

auto wall::SignalHandler::stop_listening() -> void {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
}
