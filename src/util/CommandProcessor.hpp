#pragma once

#include "conf/Config.hpp"
#include "util/Loop.hpp"

namespace wall {
namespace commands {
constexpr auto k_lock = "lock";
constexpr auto k_wallpaper = "wallpaper";
constexpr auto k_stop = "stop";
constexpr auto k_next = "next";
constexpr auto k_reload = "reload";
constexpr auto k_full_reload = "full_reload";
}  // namespace commands

class Wallock;

class CommandProcessor {
   public:
    CommandProcessor(Wallock* wallock);

    virtual ~CommandProcessor();

    auto start_listening() -> bool;

    auto stop_listening() -> void;

    static auto send(const Config& config, const std::string& cmd) -> void;

    static auto is_running(const Config& config) -> bool;

    static auto get_socket_filename(const Config& config) -> std::filesystem::path;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    auto process(const std::string& cmd) -> void;

   private:
    const Config& m_config;

    Loop* m_loop{};

    loop::UnixSocket* m_pipe{};

    Wallock* m_wallock{};

    std::string m_socket_filename;

    bool m_is_ignore_running{false};
};
}  // namespace wall
