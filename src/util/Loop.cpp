#include "util/Loop.hpp"

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <limits>
#include <thread>
#include "util/Log.hpp"

namespace {
std::atomic_uint64_t g_handle_id{1UL};
}

wall::loop::Handle::Handle() : m_id{g_handle_id++} {}

wall::loop::Handle::~Handle() = default;

auto wall::loop::Handle::close() -> void { m_is_closing = true; }

auto wall::loop::Handle::is_closing() const -> bool { return m_is_closing; }

auto wall::loop::Handle::get_id() const -> uint64_t { return m_id; }

wall::loop::Poll::Poll(int32_t file_descriptor, int16_t trigger_events, std::function<void(Poll*, int16_t)> callback)
    : m_fd{file_descriptor}, m_trigger_events{trigger_events}, m_callback{std::move(callback)} {}

auto wall::loop::Poll::get_fd() const -> int32_t { return m_fd; }

auto wall::loop::Poll::set_fd(int32_t file_descriptor) -> void { m_fd = file_descriptor; }

auto wall::loop::Poll::set_trigger_events(int16_t events) -> void { m_trigger_events = events; }

auto wall::loop::Poll::get_trigger_events() const -> int16_t { return m_trigger_events; }

auto wall::loop::Poll::trigger(int16_t events) -> void { m_callback(this, events); }

wall::loop::PollPipe::PollPipe(std::function<void(PollPipe*, const std::vector<uint8_t>& buffer)> callback)
    : Poll{0, POLLIN, [this](Poll*, uint16_t events) { this->callback_wrapper(events); }}, m_callback{std::move(callback)} {
    std::array<int, 2> fds;
    if (pipe(fds.data()) == -1) {
        LOG_FATAL("Failed to create pipe");
        return;
    }
    set_fd(fds[0]);
    m_write_fd = fds[1];
}

wall::loop::PollPipe::~PollPipe() {
    // close fds
    if (get_fd() != -1) {
        ::close(get_fd());
    }
    if (m_write_fd != -1) {
        ::close(m_write_fd);
    }
}

auto wall::loop::PollPipe::callback_wrapper(int16_t events) -> void {
    if ((events & POLLIN) != 0) {
        std::vector<uint8_t> buffer;
        buffer.resize(1024);
        const auto read_bytes = ::read(get_fd(), buffer.data(), buffer.size());
        if (read_bytes == -1) {
            LOG_ERROR("Failed to read from pipe");
            return;
        }
        buffer.resize(read_bytes);

        m_callback(this, buffer);
    } else {
        LOG_ERROR("Invalid events for pipe {}", events);
    }
}

auto wall::loop::PollPipe::write(uint8_t* buffer, size_t size) const -> void {
    // write buffer to fd
    while (size > 0) {
        const auto written = ::write(m_write_fd, buffer, size);
        if (written == -1) {
            LOG_ERROR("Failed to write to pipe");
            return;
        }
        size -= written;
        buffer += written;
    }
}

auto wall::loop::PollPipe::write_one() const -> void { write_one(1); }

auto wall::loop::PollPipe::write_one(uint8_t one) const -> void { PollPipe::write(&one, 1); }

wall::loop::Timer::Timer(std::chrono::system_clock::time_point expiration, std::chrono::milliseconds interval, std::function<void(Timer*)> callback)
    : m_expiration{expiration}, m_interval{interval}, m_callback{std::move(callback)} {}

auto wall::loop::Timer::trigger() -> void { m_callback(this); }

auto wall::loop::Timer::get_expiration() const -> std::chrono::system_clock::time_point { return m_expiration; }

auto wall::loop::Timer::set_expiration(std::chrono::system_clock::time_point expiration) -> void { m_expiration = expiration; }

auto wall::loop::Timer::get_interval() const -> std::chrono::milliseconds { return m_interval; }

auto wall::loop::Timer::set_interval(std::chrono::milliseconds interval) -> void { m_interval = interval; }

wall::loop::UnixSocket::UnixSocket(Loop* loop, const std::filesystem::path& path, std::function<void(UnixSocket*, const std::string&)> callback)
    : Poll{0, POLLIN, [this](Poll*, uint16_t events) { accept_new_connection(events); }},
      m_loop{loop},
      m_path{path},
      m_callback{std::move(callback)} {
    // Create a socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_FATAL("Error creating socket at {} {}", path.string(), strerror(errno));
    }
    set_fd(sockfd);

    // Connect to the Unix socket
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    std::strncpy(server_addr.sun_path, path.c_str(), sizeof(server_addr.sun_path) - 1);

    unlink(path.c_str());  // Remove the existing file, if any
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_FATAL("Error binding to socket at {} {}", path.string(), strerror(errno));
    }

    // Listen for connections (if server-side)
    if (listen(sockfd, 1) < 0) {
        LOG_FATAL("Error listening to socket at {} {}", path.string(), strerror(errno));
    }
}

wall::loop::UnixSocket::~UnixSocket() {
    // close socket
    if (get_fd() != -1) {
        ::close(get_fd());
    }

    for (const auto& [id, client] : m_clients) {
        client->close();
        ::close(client->get_fd());
    }
}

auto wall::loop::UnixSocket::get_path() const -> std::filesystem::path { return m_path; }

auto wall::loop::UnixSocket::accept_new_connection(int16_t events) -> void {
    if ((events & POLLIN) == 0) {
        LOG_ERROR("Invalid events for socket {}", events);
        return;
    }

    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    const auto client_fd = accept(get_fd(), (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        LOG_ERROR("Failed to accept new connection {}", strerror(errno));
        return;
    }

    auto* client_poll = m_loop->add_poll(client_fd, POLLIN, [this](Poll* socket, int16_t events2) { client_read_callback(socket, events2); });
    m_clients[client_poll->get_id()] = client_poll;
}

auto wall::loop::UnixSocket::client_read_callback(Poll* poll, int16_t events) -> void {
    if ((events & POLLIN) != 0) {
        std::string buffer;
        buffer.resize(1024);
        const auto read_bytes = ::read(poll->get_fd(), buffer.data(), buffer.size());
        if (read_bytes == 0) {
            poll->close();
            ::close(poll->get_fd());
            m_clients.erase(poll->get_id());
            return;
        }

        if (read_bytes < 0) {
            LOG_ERROR("Failed to read from socket {}", strerror(errno));
            return;
        }
        buffer.resize(read_bytes);

        m_callback(this, buffer);
    } else {
        LOG_ERROR("Invalid events for socket {}", events);
    }
}

wall::Loop::Loop() : m_now{std::chrono::system_clock::now()} {};

wall::Loop::~Loop() = default;

auto wall::Loop::run() -> bool {
    auto is_any_open = std::any_of(m_handles.begin(), m_handles.end(), [](const auto& handle) { return !handle->is_closing(); });
    if (!is_any_open) {
        return false;
    }

    // calculate min timer expiration
    std::chrono::milliseconds min_timeout = calculate_timeout();

    // poll fds with timer expiration as max timeout
    poll_fds(min_timeout);

    // run close handlers first
    std::erase_if(m_handles, [](const auto& handle) { return handle->is_closing(); });

    m_now = std::chrono::system_clock::now();

    // check timers
    handle_timers(m_now);

    // check signals
    return true;
}

auto wall::Loop::calculate_timeout() const -> std::chrono::milliseconds {
    std::chrono::milliseconds min_timeout{std::numeric_limits<int32_t>::max()};
    for (const auto& handle : m_handles) {
        if (!handle->is_closing() && handle->get_type() == loop::HandleType::Timer) {
            auto* timer = static_cast<loop::Timer*>(handle.get());
            const auto timer_duration = std::chrono::duration_cast<std::chrono::milliseconds>(timer->get_expiration() - m_now);
            if (timer_duration < min_timeout) {
                min_timeout = timer_duration;
            }
        }
    }

    return min_timeout;
}

auto wall::Loop::add_poll(int32_t file_descriptor, int16_t trigger_events, std::function<void(loop::Poll*, int16_t)> callback) -> loop::Poll* {
    return add_handle(std::make_unique<loop::Poll>(file_descriptor, trigger_events, std::move(callback)));
}

auto wall::Loop::add_poll_pipe(std::function<void(loop::PollPipe*, const std::vector<uint8_t>&)> callback) -> loop::PollPipe* {
    return add_handle(std::make_unique<loop::PollPipe>(std::move(callback)));
}

auto wall::Loop::add_timer(std::chrono::milliseconds initial_delay, std::chrono::milliseconds interval, std::function<void(loop::Timer*)> callback)
    -> loop::Timer* {
    return add_handle(std::make_unique<loop::Timer>(m_now + initial_delay, interval, std::move(callback)));
}

auto wall::Loop::add_unix_socket(const std::filesystem::path& path, std::function<void(loop::UnixSocket*, const std::string&)> callback)
    -> loop::UnixSocket* {
    return add_handle(std::make_unique<loop::UnixSocket>(this, path, std::move(callback)));
}

auto wall::Loop::handle_timers(std::chrono::system_clock::time_point now) -> void {
    // The m_handles vector might change on triggering the timer, so we have to check the vector each time
    while (true) {
        loop::Timer* valid_timer = nullptr;
        for (const auto& handle : m_handles) {
            if (!handle->is_closing() && handle->get_type() == loop::HandleType::Timer) {
                auto* check_timer = static_cast<loop::Timer*>(handle.get());
                if (check_timer->get_expiration() <= now) {
                    valid_timer = check_timer;
                    break;
                }
            }
        }

        if (valid_timer == nullptr) {
            break;
        }

        // set next expiration
        if (valid_timer->get_interval().count() > 0) {
            valid_timer->set_expiration(now + valid_timer->get_interval());
        } else {
            valid_timer->set_expiration(std::chrono::system_clock::time_point::max());
        }
        valid_timer->trigger();
    }
}

auto wall::Loop::poll_fds(std::chrono::milliseconds min_timeout) -> void {
    std::vector<pollfd> fds;
    std::vector<loop::Poll*> polls;
    for (const auto& handle : m_handles) {
        if (!handle->is_closing() && (handle->get_type() == loop::HandleType::Poll || handle->get_type() == loop::HandleType::PollPipe ||
                                      handle->get_type() == loop::HandleType::UnixSocket)) {
            auto* poll = static_cast<loop::Poll*>(handle.get());
            const int16_t events = poll->get_trigger_events() | POLLERR | POLLHUP;  // always listen for errors and hangups
            fds.push_back({poll->get_fd(), events, 0});
            polls.push_back(poll);
        }
    }

    if (!fds.empty()) {
        poll(fds.data(), fds.size(), min_timeout.count());
        for (auto i = 0UL; i < fds.size(); i++) {
            auto* poll_handle = polls[i];
            if (!poll_handle->is_closing() && (fds[i].revents & poll_handle->get_trigger_events()) != 0) {
                polls[i]->trigger(fds[i].revents);
            }
        }
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds{min_timeout});
    }
}

auto wall::Loop::print_open_handles() -> void {
    for (const auto& handle : m_handles) {
        if (!handle->is_closing()) {
            LOG_DEBUG("Handle: {} type {}", handle->get_id(), static_cast<int>(handle->get_type()));
        }
    }
}
