#pragma once

#include <wayland-client-protocol.h>
#include <wayland-client.h>

namespace wall {
class Subcompositor {
   public:
    Subcompositor(wl_subcompositor* subcompositor);
    virtual ~Subcompositor();

    Subcompositor(const Subcompositor&) = delete;
    auto operator=(const Subcompositor&) -> Subcompositor& = delete;

    Subcompositor(Subcompositor&&) = delete;
    auto operator=(Subcompositor&&) -> Subcompositor& = delete;

    auto set_wl_subcompositor(wl_subcompositor* subcompositor) -> void;

    [[nodiscard]] auto get_wl_subcompositor() const -> wl_subcompositor*;

   protected:
   private:
    wl_subcompositor* m_subcompositor{};
};
}  // namespace wall
