#include "registry/XdgBase.hpp"

#include <cstdint>

#include "xdg-shell-protocol.h"

struct xdg_wm_base;

const xdg_wm_base_listener wall::XdgBase::k_listener = {
    .ping =
        [](void* data, xdg_wm_base* xdg_wm_base, uint32_t serial) {
            [[maybe_unused]] auto& self = *static_cast<wall::XdgBase*>(data);
            xdg_wm_base_pong(xdg_wm_base, serial);
        },
};

wall::XdgBase::XdgBase(xdg_wm_base* base) : m_base{base} {
    if (m_base == nullptr) {
        return;
    }

    xdg_wm_base_add_listener(m_base, &k_listener, this);
}

wall::XdgBase::~XdgBase() {
    if (m_base != nullptr) {
        xdg_wm_base_destroy(m_base);
    }
}

auto wall::XdgBase::set_wm_base(xdg_wm_base* base) -> void {
    if (m_base != nullptr) {
        xdg_wm_base_destroy(m_base);
    }

    m_base = base;
    xdg_wm_base_add_listener(m_base, &k_listener, this);
}
