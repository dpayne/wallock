#include "registry/Lock.hpp"

#include <wayland-client-protocol.h>
#include "util/Log.hpp"

const ext_session_lock_v1_listener wall::Lock::k_listener = {
    .locked =
        [](void* data, ext_session_lock_v1* /* lock */) {
            auto& self = *static_cast<wall::Lock*>(data);
            self.on_locked();
        },
    .finished =
        [](void* data, ext_session_lock_v1* /* lock */) {
            auto& self = *static_cast<wall::Lock*>(data);
            self.finished();
        },
};

wall::Lock::Lock(ext_session_lock_v1* lock) : m_lock{lock} {
    if (m_lock == nullptr) {
        return;
    }

    ext_session_lock_v1_add_listener(m_lock, &k_listener, this);
}

auto wall::Lock::get_lock() -> ext_session_lock_v1* { return m_lock; }

auto wall::Lock::is_locked() const -> bool { return m_is_locked; }

auto wall::Lock::trigger_unlock() -> void {
    if (m_lock == nullptr) {
        return;
    }
    ext_session_lock_v1_unlock_and_destroy(m_lock);
    m_lock = nullptr;
}

auto wall::Lock::on_locked() -> void {
    LOG_DEBUG("Locked");
    m_is_locked = true;
}

auto wall::Lock::finished() -> void {
    m_is_locked = false;
    LOG_FATAL("Failed to lock session, is another lock active?");
}
