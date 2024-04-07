#include "registry/Subcompositor.hpp"

#include <wayland-client-protocol.h>

struct wl_subcompositor;

wall::Subcompositor::Subcompositor(wl_subcompositor* subcompositor) : m_subcompositor{subcompositor} {
    if (m_subcompositor == nullptr) {
        return;
    }
}

wall::Subcompositor::~Subcompositor() {
    if (m_subcompositor != nullptr) {
        wl_subcompositor_destroy(m_subcompositor);
    }
}

auto wall::Subcompositor::set_wl_subcompositor(wl_subcompositor* subcompositor) -> void {
    if (m_subcompositor != nullptr) {
        wl_subcompositor_destroy(m_subcompositor);
    }

    m_subcompositor = subcompositor;
}

auto wall::Subcompositor::get_wl_subcompositor() const -> wl_subcompositor* { return m_subcompositor; }
