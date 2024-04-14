#include "util/CommandProcessor.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "conf/ConfigMacros.hpp"
#include "util/FileUtils.hpp"
#include "util/StringUtils.hpp"
#include "wallock/Wallock.hpp"

wall::CommandProcessor::CommandProcessor(Wallock* wallock) : m_config{wallock->get_config()}, m_loop{wallock->get_loop()}, m_wallock{wallock} {
    m_socket_filename = StringUtils::trim(wall_conf_get(get_config(), command, socket_filename));
    m_is_ignore_running = wall_conf_get(get_config(), command, ignore_is_running);
}

wall::CommandProcessor::~CommandProcessor() { stop_listening(); }

auto wall::CommandProcessor::get_config() const -> const Config& { return m_config; }

auto wall::CommandProcessor::get_socket_filename(const Config& config) -> std::filesystem::path {
    return FileUtils::get_default_runtime_dir() / StringUtils::trim(wall_conf_get(config, command, socket_filename));
}

auto wall::CommandProcessor::start_listening() -> bool {
    if (m_pipe != nullptr) {
        LOG_ERROR("Already listening");
        return false;
    }

    const auto socket_filename = get_socket_filename(get_config());
    if (!m_is_ignore_running && is_running(get_config())) {
        LOG_ERROR("Already running, if this is a mistake, please remove the file: {}", socket_filename.string());
        return false;
    }

    std::filesystem::create_directories(socket_filename.parent_path());

    if (std::filesystem::exists(socket_filename)) {
        LOG_ERROR("Socket already exists: {}", socket_filename.string());
        std::filesystem::remove(socket_filename);
    }

    m_pipe = m_loop->add_unix_socket(socket_filename, [this](loop::UnixSocket*, const std::string& msg) { this->process(msg); });
    return true;
}

auto wall::CommandProcessor::send(const Config& config, const std::string& cmd) -> void {
    const auto socket_filename = FileUtils::get_default_runtime_dir() / StringUtils::trim(wall_conf_get(config, command, socket_filename));
    if (!std::filesystem::exists(socket_filename)) {
        LOG_ERROR("Socket does not exist: {}", socket_filename.string());
    }

    // Create a socket
    const auto sockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_FATAL("Error creating socket at {}", socket_filename.string());
    }

    // Connect to the Unix socket
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    std::strncpy(server_addr.sun_path, socket_filename.c_str(), sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_FATAL("Error connecting to socket at {}", socket_filename.string());
    }

    // Send the command
    const auto written = ::write(sockfd, cmd.c_str(), cmd.size());
    if (written == -1) {
        LOG_ERROR("Failed to write to socket");
    }

    // Close the socket
    ::close(sockfd);
}

auto wall::CommandProcessor::stop_listening() -> void {
    if (m_pipe != nullptr) {
        const auto socket_filename = m_pipe->get_path();
        m_pipe->close();
        m_pipe = nullptr;

        if (std::filesystem::exists(socket_filename)) {
            std::filesystem::remove(socket_filename);
        }
    }
}

auto wall::CommandProcessor::process(const std::string& cmd) -> void {
    // remove whitespace
    auto cmd_no_ws = cmd;
    cmd_no_ws.erase(std::remove_if(cmd_no_ws.begin(), cmd_no_ws.end(), ::isspace), cmd_no_ws.end());

    // remove newline
    cmd_no_ws.erase(std::remove(cmd_no_ws.begin(), cmd_no_ws.end(), '\n'), cmd_no_ws.end());

    if (cmd_no_ws == commands::k_lock) {
        m_wallock->lock();
    } else if (cmd_no_ws == commands::k_stop) {
        m_wallock->stop();
    } else if (cmd_no_ws == commands::k_next) {
        m_wallock->next();
    } else if (cmd_no_ws == commands::k_full_reload) {
        m_wallock->full_reload();
    } else if (cmd_no_ws == commands::k_reload) {
        m_wallock->reload();
    } else {
        LOG_ERROR("Unknown command: {}", cmd);
    }
}

auto wall::CommandProcessor::is_running(const Config& config) -> bool {
    const auto socket_filename = FileUtils::get_default_runtime_dir() / StringUtils::trim(wall_conf_get(config, command, socket_filename));
    return std::filesystem::is_socket(socket_filename);
}
