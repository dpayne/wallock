#pragma once

#include "conf/Config.hpp"
namespace wall {
class Wallock;
class SignalHandler {
   public:
    SignalHandler(const Config* config);
    ~SignalHandler();

    SignalHandler(const SignalHandler& other) = delete;
    auto operator=(const SignalHandler& other) -> SignalHandler& = delete;
    SignalHandler(SignalHandler&& other) = delete;
    auto operator=(SignalHandler&& other) -> SignalHandler& = delete;

    static auto stop_listening() -> void;

   protected:
    static auto shutdown_handler(int signal) -> void;

    static auto refresh_handler(int signal) -> void;

    static auto reload_handler(int signal) -> void;

   private:
};
}  // namespace wall
