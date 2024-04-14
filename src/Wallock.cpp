#include "wallock/Wallock.hpp"

#include <wayland-util.h>
#include <cstdlib>
#include "conf/Config.hpp"
#include "conf/ConfigMacros.hpp"
#include "conf/ConfigValidator.hpp"
#include "display/Display.hpp"
#include "util/CommandProcessor.hpp"
#include "util/Log.hpp"
#include "util/SignalHandler.hpp"
#include "util/StringUtils.hpp"

auto wall::Wallock::start(int argc, const char** argv) -> void {
    // first validate the config
    wall::Config test_config{argc, argv};
    if (!ConfigValidator::validate(test_config)) {
        exit(1);
        return;
    }

    // deamonize check needs to happen before we initialize the logger or do anything else
    daemonize(argc, argv);

    std::srand(std::time(nullptr));

    wall::Config config{argc, argv};
    if (config.is_debug()) {
        wall::Log::setup_debug_logger(config);
    } else {
        wall::Log::setup_default_logger(config);
    }

    Loop loop;

    auto cmd = StringUtils::trim(wall_conf_get(config, command, name));
    wall::Wallock wallock{&config, &loop};

    if (cmd.empty()) {
        start_wallock(config, &wallock);
    } else if (CommandProcessor::is_running(config)) {
        CommandProcessor::send(config, std::string{cmd});
    } else {
        LOG_FATAL("Command specified but no instance is running. Run without a command to start a new instance first.");
    }

    wall::Log::__flush();
}

auto wall::Wallock::daemonize(int argc, const char** argv) -> void {
    auto is_deamonize = true;
    for (auto arg_ix = 1; arg_ix < argc; arg_ix++) {
        if (Config::is_exit_immediately_option(argv[arg_ix])) {
            is_deamonize = false;
            break;
        }
    }

    if (is_deamonize) {
        LOG_DEBUG("Daemonizing...");
        if (daemon(0, 0) == -1) {
            LOG_FATAL("Failed to daemonize");
            return;
        }
    }
}

wall::Wallock::Wallock(Config* config, Loop* loop) : m_config{config}, m_loop{loop} {}

auto wall::Wallock::start_wallock(const Config& config, Wallock* wallock) -> void {
    if (wall_conf_get(config, start, lock)) {
        wallock->run(true);
    } else if (wall_conf_get(config, wallpaper, enabled)) {
        wallock->run(false);
    } else {
        LOG_WARN("No command specified and wallpaper is disabled, assuming lock command");
        wallock->run(true);
    }
}

auto wall::Wallock::get_config() const -> const Config& { return *m_config; }

auto wall::Wallock::get_command_processor_mut() const -> wall::CommandProcessor* { return m_command_processor.get(); }

auto wall::Wallock::run(bool is_lock) -> void {
    m_is_alive = true;
    while (m_is_alive) {
        // On full_reload, we repeat the loop

        LOG_DEBUG("Running...");
        m_display = nullptr;

        m_command_processor = std::make_unique<wall::CommandProcessor>(this);
        m_signal_handler = std::make_unique<wall::SignalHandler>(&get_config());
        if (!m_command_processor->start_listening()) {
            LOG_FATAL("Failed to start listening");
            return;
        }

        m_display = std::make_unique<wall::Display>(get_config(), m_loop, is_lock, [this]() {
            m_command_processor = nullptr;
            m_signal_handler = nullptr;
        });

        m_display->loop();
        LOG_DEBUG("loop done");

        if (!m_is_full_reloading) {
            // we exited the loop and we are not reloading, so we are done
            break;
        }

        // reset the flag
        m_is_full_reloading = false;
        is_lock = false;  // full reload is always a wallpaper reload
    }
}

auto wall::Wallock::lock() -> void {
    if (m_display->is_locked()) {
        LOG_WARN("Already locked");
    } else {
        LOG_DEBUG("Locking...");
        m_display->lock();
    }
}

auto wall::Wallock::stop() -> void {
    m_is_alive = false;

    if (m_display == nullptr) {
        return;
    }

    if (m_display->is_locked()) {
        LOG_ERROR("Cannot stop while locked");
    } else {
        LOG_DEBUG("Stopping...");
        m_display->stop();
    }
}

auto wall::Wallock::get_loop() const -> wall::Loop* { return m_loop; }

auto wall::Wallock::next() -> void {
    if (m_display == nullptr) {
        return;
    }

    m_display->next();
}

auto wall::Wallock::reload() -> void {
    if (m_display == nullptr) {
        return;
    }

    m_config->reload_options();
    m_display->reload();
}

auto wall::Wallock::full_reload() -> void {
    if (m_display == nullptr) {
        return;
    }

    if (m_display->is_locked()) {
        LOG_ERROR("Cannot do a full reload while locked, try reload command instead");
        return;
    }

    m_is_full_reloading = true;

    LOG_DEBUG("Fully reloading...");
    m_display->stop();

    m_config->reload_options();

    // we will continue in the run loop
}
