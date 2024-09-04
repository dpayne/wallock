#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>

namespace wall {

namespace loop {
enum class HandleType {
    None,
    Poll,
    PollPipe,
    UnixSocket,
    TcpSocket,
    UdpSocket,
    Timer,
    Signal,
};
class Handle;
class Poll;
class PollPipe;
class UnixSocket;
class Timer;
}  // namespace loop

class Loop {
   public:
    Loop();
    virtual ~Loop();

    Loop(Loop&&) = delete;
    Loop(const Loop&) = delete;
    auto operator=(const Loop&) -> Loop = delete;
    auto operator=(Loop&&) -> Loop = delete;

    auto run() -> bool;

    auto print_open_handles() -> void;

    auto add_poll(int32_t file_descriptor, int16_t trigger_events, std::function<void(loop::Poll*, int16_t)> callback) -> loop::Poll*;

    auto add_poll_pipe(std::function<void(loop::PollPipe*, const std::vector<uint8_t>&)> callback) -> loop::PollPipe*;

    auto add_unix_socket(const std::filesystem::path& path, std::function<void(loop::UnixSocket*, const std::string&)> callback) -> loop::UnixSocket*;

    auto add_timer(std::chrono::milliseconds initial_delay,
                   std::chrono::milliseconds interval,
                   std::function<void(loop::Timer*)> callback) -> loop::Timer*;

   protected:
    auto poll_fds(std::chrono::milliseconds min_timeout) -> void;

    auto handle_timers(std::chrono::system_clock::time_point now) -> void;

    [[nodiscard]] auto calculate_timeout() const -> std::chrono::milliseconds;

    template <std::derived_from<loop::Handle> HandleType>
    auto add_handle(std::unique_ptr<HandleType> handle) -> HandleType* {
        auto* retval = handle.get();
        m_handles.push_back(std::move(handle));
        return retval;
    }

   private:
    std::vector<std::unique_ptr<loop::Handle>> m_handles;

    std::chrono::system_clock::time_point m_now;
};

namespace loop {

class Handle {
   public:
    explicit Handle();
    virtual ~Handle();

    Handle(Handle&&) = delete;
    Handle(const Handle&) = delete;
    auto operator=(const Handle&) -> Handle = delete;
    auto operator=(Handle&&) -> Handle = delete;

    [[nodiscard]] auto get_id() const -> uint64_t;

    auto close() -> void;

   protected:
    [[nodiscard]] auto is_closing() const -> bool;

    [[nodiscard]] virtual auto get_type() const -> HandleType { return HandleType::None; }

   private:
    bool m_is_closing{false};

    uint64_t m_id{0UL};

    friend class ::wall::Loop;
};

class Timer : public Handle {
   public:
    explicit Timer(std::chrono::system_clock::time_point expiration, std::chrono::milliseconds interval, std::function<void(Timer*)> callback);

    [[nodiscard]] auto get_type() const -> HandleType override { return HandleType::Timer; }

    [[nodiscard]] auto get_interval() const -> std::chrono::milliseconds;

    auto set_interval(std::chrono::milliseconds interval) -> void;

    [[nodiscard]] auto get_expiration() const -> std::chrono::system_clock::time_point;

    auto set_expiration(std::chrono::system_clock::time_point expiration) -> void;

   protected:
    auto trigger() -> void;

   private:
    std::chrono::system_clock::time_point m_expiration;

    std::chrono::milliseconds m_interval;

    std::function<void(Timer*)> m_callback;

    friend class ::wall::Loop;
};

class Poll : public Handle {
   public:
    explicit Poll(int32_t file_descriptor, int16_t trigger_events, std::function<void(Poll*, int16_t)> callback);

    [[nodiscard]] auto get_type() const -> HandleType override { return HandleType::Poll; }

    [[nodiscard]] auto get_fd() const -> int32_t;

    [[nodiscard]] auto get_trigger_events() const -> int16_t;

   protected:
    auto trigger(int16_t) -> void;

    auto set_fd(int32_t file_descriptor) -> void;

    auto set_trigger_events(int16_t events) -> void;

   private:
    int32_t m_fd{0};

    int16_t m_trigger_events{0};

    std::function<void(Poll*, int16_t)> m_callback;

    friend class ::wall::Loop;
};

class PollPipe : public Poll {
   public:
    explicit PollPipe(std::function<void(PollPipe*, const std::vector<uint8_t>& buffer)> callback);

    ~PollPipe() override;

    auto write(uint8_t* buffer, size_t size) const -> void;

    auto write_one() const -> void;

    auto write_one(uint8_t one) const -> void;

    [[nodiscard]] auto get_type() const -> HandleType override { return HandleType::PollPipe; }

   protected:
    auto callback_wrapper(int16_t events) -> void;

   private:
    int32_t m_write_fd;

    std::function<void(PollPipe*, const std::vector<uint8_t>& buffer)> m_callback;

    friend class ::wall::Loop;
};

class UnixSocket : public Poll {
   public:
    explicit UnixSocket(Loop* loop, const std::filesystem::path& path, std::function<void(UnixSocket*, const std::string&)> callback);

    ~UnixSocket() override;

    [[nodiscard]] auto get_path() const -> std::filesystem::path;

    [[nodiscard]] auto get_type() const -> HandleType override { return HandleType::UnixSocket; }

   protected:
    auto client_read_callback(Poll* poll, int16_t events) -> void;

    auto accept_new_connection(int16_t events) -> void;

   private:
    Loop* m_loop{};

    std::filesystem::path m_path;

    std::function<void(UnixSocket*, const std::string&)> m_callback;

    std::unordered_map<uint64_t, loop::Poll*> m_clients;

    friend class ::wall::Loop;
};

}  // namespace loop
}  // namespace wall
