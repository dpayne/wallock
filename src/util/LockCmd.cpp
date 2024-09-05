#include "util/LockCmd.hpp"
#include <csignal>

#include "util/Log.hpp"

wall::LockCmd::LockCmd(const Config& config) : config(config) {}

auto wall::LockCmd::get_config() const -> const Config& { return config; }

// This code was largely lifted from swaylock https://github.com/swaywm/swaylock/pull/350/files
auto wall::LockCmd::run() -> void {
    const auto lock_cmd = wall_conf_get(get_config(), general, lock_cmd);
    if (lock_cmd.empty()) {
        return;
    }

    LOG_DEBUG("Running lock command: {}", lock_cmd);
    if (!lock_cmd.empty()) {
        m_lock_process = fork();
        if (m_lock_process == 0) {
            execl("/bin/sh", "/bin/sh", "-c", lock_cmd.data(), nullptr);
        } else if (m_lock_process == -1) {
            LOG_ERROR("Unable to fork: {}", strerror(errno));
        }
    }
}

auto wall::LockCmd::stop() -> void {
    if (m_lock_process != -1) {
        LOG_DEBUG("Stopping lock command");
        kill(m_lock_process, SIGTERM);
        m_lock_process = -1;
    }
}
