#include "registry/Compositor.hpp"

#include <wayland-client-protocol.h>

struct wl_compositor;

wall::Compositor::Compositor(wl_compositor* compositor) : m_compositor{compositor} {
    if (m_compositor == nullptr) {
        return;
    }
}

wall::Compositor::~Compositor() {
    if (m_compositor != nullptr) {
        wl_compositor_destroy(m_compositor);
    }
}

auto wall::Compositor::set_wl_compositor(wl_compositor* compositor) -> void {
    if (m_compositor != nullptr) {
        wl_compositor_destroy(m_compositor);
    }

    m_compositor = compositor;
}

auto wall::Compositor::get_wl_compositor() const -> wl_compositor* { return m_compositor; }
