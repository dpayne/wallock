#include "util/SignalHandler.hpp"
#include <csignal>

#include "util/CommandProcessor.hpp"
#include "util/Log.hpp"
#include "wallock/Wallock.hpp"

namespace {
const wall::Config* g_config = nullptr;
wall::Wallock* g_instance = nullptr;
}  // namespace

wall::SignalHandler::SignalHandler(const Config* config, Wallock* wallock) {
    if (g_config == nullptr) {
        g_config = config;
        g_instance = wallock;
    } else {
        LOG_WARN("SignalHandler already initialized");
    }

    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);
    signal(SIGQUIT, shutdown_handler);
    signal(SIGHUP, shutdown_handler);
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
    if (g_instance != nullptr) {
        g_instance->stop();
    }
    g_config = nullptr;
    g_instance = nullptr;
}

auto wall::SignalHandler::stop_listening() -> void {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
}
