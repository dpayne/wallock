#include "util/OnKeyProcessor.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>
#include <utility>

#include "State.hpp"
#include "conf/ConfigMacros.hpp"
#include "pam/PasswordManager.hpp"
#include "util/PasswordBuffer.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::OnKeyProcessor::OnKeyProcessor(const Config& config,
                                     std::unique_ptr<PasswordManager> password_manager,
                                     std::function<void(State)> on_state_change)
    : m_config{config},
      m_password_manager{std::move(password_manager)},
      m_on_state_change{std::move(on_state_change)},
      m_max_password_size{wall_conf_get(config, password, max_length)},
      m_password_buffer{std::make_unique<PasswordBuffer>(m_max_password_size)} {}

auto wall::OnKeyProcessor::get_config() const -> const Config& { return m_config; }

auto wall::OnKeyProcessor::stop() -> void { m_password_manager->stop(); }

auto wall::OnKeyProcessor::on_key(xkb_keysym_t keysym, const std::string& utf8_key) -> void {
    switch (keysym) {
        case XKB_KEY_NoSymbol:
            m_on_state_change(State::NoOp);
            break;
        case XKB_KEY_KP_Enter:
            [[fallthrough]];
        case XKB_KEY_Return:
            handle_enter_key();
            break;
        case XKB_KEY_Caps_Lock:
            m_on_state_change(State::CapsLock);
            break;
        case XKB_KEY_Delete:
            [[fallthrough]];
        case XKB_KEY_BackSpace:
            handle_backspace();
            break;
        case XKB_KEY_Escape:
            handle_clear_key();
            break;
        default:
            if (utf8_key.empty()) {
                m_on_state_change(State::NoOp);
                break;
            }

            m_password_buffer->push(utf8_key);
            m_on_state_change(State::Keypress);
            break;
    }
}

auto wall::OnKeyProcessor::get_password_buffer() const -> PasswordBuffer* { return m_password_buffer.get(); }

auto wall::OnKeyProcessor::handle_backspace() -> void {
    m_password_buffer->pop();

    if (!m_password_buffer->empty()) {
        m_on_state_change(State::Backspace);
    } else {
        m_on_state_change(State::Cleared);
    }
}

auto wall::OnKeyProcessor::handle_clear_key() -> void {
    m_password_buffer->clear();
    m_on_state_change(State::Cleared);
}

auto wall::OnKeyProcessor::handle_enter_key() -> void {
    // skip empty password
    if (m_password_buffer->empty() && !wall_conf_get(get_config(), password, allow_empty)) {
        return;
    }

    m_on_state_change(State::Verifying);
    m_password_manager->authenticate(std::move(m_password_buffer), m_on_state_change);
    m_password_buffer = std::make_unique<PasswordBuffer>(m_max_password_size);
}
