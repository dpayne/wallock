#pragma once

#include <memory>

namespace wall {

class Display;
class Loop;
class Config;
class SignalHandler;
class CommandProcessor;

/**
 * @brief Wallock */
class Wallock {
   public:
    static auto start(int argc, const char** argv) -> void;

    Wallock(Config* config, Loop* loop);
    virtual ~Wallock() = default;

    Wallock(Wallock&&) = delete;
    Wallock(const Wallock&) = delete;
    auto operator=(const Wallock&) -> Wallock = delete;
    auto operator=(Wallock&&) -> Wallock = delete;

    virtual auto run(bool is_lock) -> void;

    virtual auto next() -> void;

    virtual auto stop() -> void;

    virtual auto full_reload() -> void;

    virtual auto reload() -> void;

    virtual auto lock() -> void;

    [[nodiscard]] auto get_loop() const -> Loop*;

    [[nodiscard]] auto get_config() const -> const Config&;

   protected:
    [[nodiscard]] auto get_command_processor_mut() const -> CommandProcessor*;

    static auto start_wallock(const Config& config, Wallock* wallock) -> void;

    static auto daemonize(int argc, const char** argv) -> void;

   private:
    Config* m_config;

    bool m_is_alive{false};

    bool m_is_full_reloading{false};

    Loop* m_loop{nullptr};

    std::unique_ptr<SignalHandler> m_signal_handler{nullptr};

    std::unique_ptr<CommandProcessor> m_command_processor{nullptr};

    std::unique_ptr<Display> m_display{nullptr};
};

}  // namespace wall
