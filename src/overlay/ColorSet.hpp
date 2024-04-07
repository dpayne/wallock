#pragma once

#include "State.hpp"
#include "overlay/Color.hpp"

namespace wall {
struct ColorSet {
    Color m_input;
    Color m_cleared;
    Color m_caps_lock;
    Color m_verifying;
    Color m_wrong;

    [[nodiscard]] auto get(State state) const -> Color;
};
}  // namespace wall
