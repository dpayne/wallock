#pragma once

#include <wayland-client.h>
#include "ext-session-lock-v1-protocol.h"

namespace wall {
class Lock {
   public:
    explicit Lock(ext_session_lock_v1* Lock);
    virtual ~Lock() = default;

    Lock(const Lock&) = delete;
    auto operator=(const Lock&) -> Lock& = delete;

    Lock(Lock&&) = default;
    auto operator=(Lock&&) -> Lock& = default;

    [[nodiscard]] auto is_locked() const -> bool;

    auto trigger_unlock() -> void;

    auto get_lock() -> ext_session_lock_v1*;

   protected:
    auto on_locked() -> void;

    auto finished() -> void;

   private:
    static const ext_session_lock_v1_listener k_listener;

    ext_session_lock_v1* m_lock{nullptr};

    bool m_is_locked{false};
};
}  // namespace wall
