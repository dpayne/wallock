
#include "State.hpp"

auto wall::to_string(State state) -> std::string {
    switch (state) {
        case State::None:
            return "None";
        case State::NoOp:
            return "NoOp";
        case State::Idle:
            return "Idle";
        case State::Input:
            return "Input";
        case State::Cleared:
            return "Cleared";
        case State::CapsLock:
            return "CapsLock";
        case State::Verifying:
            return "Verifying";
        case State::Wrong:
            return "Wrong";
        case State::Valid:
            return "Valid";
        case State::Backspace:
            return "Backspace";
        case State::Keypress:
            return "Keypress";
    }
    return "Unknown";
}
