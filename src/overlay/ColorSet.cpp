#include "overlay/ColorSet.hpp"

#include "State.hpp"

auto wall::ColorSet::get(wall::State state) const -> wall::Color {
    switch (state) {
        case State::Input:
            return m_input;
        case State::Cleared:
            return m_cleared;
        case State::CapsLock:
            return m_caps_lock;
        case State::Verifying:
            return m_verifying;
        case State::Wrong:
            return m_wrong;
        default:
            return m_input;
    }
}
