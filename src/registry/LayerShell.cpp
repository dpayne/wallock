#include "registry/LayerShell.hpp"

#include "wlr-layer-shell-unstable-v1-protocol.h"

struct zwlr_layer_shell_v1;

wall::LayerShell::LayerShell(zwlr_layer_shell_v1* layer_shell) : m_layer_shell{layer_shell} {
    if (m_layer_shell == nullptr) {
        return;
    }
}

wall::LayerShell::~LayerShell() {
    if (m_layer_shell != nullptr) {
        zwlr_layer_shell_v1_destroy(m_layer_shell);
    }
}

auto wall::LayerShell::set_wlr_layer_shell(zwlr_layer_shell_v1* layer_shell) -> void {
    if (m_layer_shell != nullptr) {
        zwlr_layer_shell_v1_destroy(m_layer_shell);
    }

    m_layer_shell = layer_shell;
}

auto wall::LayerShell::get_wlr_layer_shell() const -> zwlr_layer_shell_v1* { return m_layer_shell; }
