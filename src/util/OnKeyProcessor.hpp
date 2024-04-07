#pragma once

#include <xkbcommon/xkbcommon.h>
#include <memory>
#include <string>
#include "State.hpp"
#include "conf/Config.hpp"
#include "pam/PasswordManager.hpp"
#include "util/PasswordBuffer.hpp"

namespace wall {
class OnKeyProcessor {
   public:
    OnKeyProcessor(const Config& config, std::unique_ptr<PasswordManager> password_manager, std::function<void(State)> on_state_change);

    OnKeyProcessor(const OnKeyProcessor&) = delete;
    OnKeyProcessor(OnKeyProcessor&&) = delete;
    auto operator=(const OnKeyProcessor&) -> OnKeyProcessor& = delete;
    auto operator=(OnKeyProcessor&&) -> OnKeyProcessor& = delete;

    auto on_key(xkb_keysym_t keysym, const std::string& utf8_key) -> void;

    auto stop() -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    auto handle_backspace() -> void;

    auto handle_clear_key() -> void;

    auto handle_enter_key() -> void;

    [[nodiscard]] auto get_password_buffer() const -> PasswordBuffer*;

   private:
    const Config& m_config;

    std::unique_ptr<PasswordManager> m_password_manager;

    std::function<void(State)> m_on_state_change;

    uint64_t m_max_password_size{};

    std::unique_ptr<PasswordBuffer> m_password_buffer;
};
}  // namespace wall
