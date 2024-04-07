#pragma once

#include "wlr-layer-shell-unstable-v1-protocol.h"

namespace wall {
class LayerShell {
   public:
    LayerShell(zwlr_layer_shell_v1* layer_shell);
    virtual ~LayerShell();

    LayerShell(const LayerShell&) = delete;
    auto operator=(const LayerShell&) -> LayerShell& = delete;

    LayerShell(LayerShell&&) = delete;
    auto operator=(LayerShell&&) -> LayerShell& = delete;

    auto set_wlr_layer_shell(zwlr_layer_shell_v1* layer_shell) -> void;

    [[nodiscard]] auto get_wlr_layer_shell() const -> zwlr_layer_shell_v1*;

   protected:
   private:
    zwlr_layer_shell_v1* m_layer_shell{};
};
}  // namespace wall
