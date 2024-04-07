#pragma once

#include <string>

namespace wall {
enum class State {
    None,
    NoOp,
    Idle,
    Input,
    Cleared,
    CapsLock,
    Verifying,
    Wrong,
    Valid,
    Backspace,
    Keypress,
};

auto to_string(State state) -> std::string;
}  // namespace wall
