#pragma once

#include "ext-session-lock-v1-protocol.h"

namespace wall {
class LockManager {
   public:
    LockManager(ext_session_lock_manager_v1* lock_manager);
    virtual ~LockManager();

    LockManager(LockManager&&) = delete;
    LockManager(const LockManager&) = delete;
    auto operator=(const LockManager&) -> LockManager = delete;
    auto operator=(LockManager&&) -> LockManager = delete;

    [[nodiscard]] auto get_lock_manager() const -> ext_session_lock_manager_v1*;

    auto set_lock_manager(ext_session_lock_manager_v1* lock_manager) -> void;

   private:
    ext_session_lock_manager_v1* m_lock_manager{};
};
}  // namespace wall
