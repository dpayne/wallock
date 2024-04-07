#include "registry/LockManager.hpp"

#include "ext-session-lock-v1-protocol.h"

struct ext_session_lock_manager_v1;

wall::LockManager::LockManager(ext_session_lock_manager_v1* lock_manager) : m_lock_manager{lock_manager} {
    if (m_lock_manager == nullptr) {
        return;
    }
}

wall::LockManager::~LockManager() {
    if (m_lock_manager != nullptr) {
        ext_session_lock_manager_v1_destroy(m_lock_manager);
    }
}

auto wall::LockManager::set_lock_manager(ext_session_lock_manager_v1* lock_manager) -> void {
    if (m_lock_manager != nullptr) {
        ext_session_lock_manager_v1_destroy(m_lock_manager);
    }

    m_lock_manager = lock_manager;
}

auto wall::LockManager::get_lock_manager() const -> ext_session_lock_manager_v1* { return m_lock_manager; }
