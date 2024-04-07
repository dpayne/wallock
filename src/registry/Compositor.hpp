#pragma once

#include <wayland-client-protocol.h>
#include <wayland-client.h>

namespace wall {
class Compositor {
   public:
    Compositor(wl_compositor* compositor);
    virtual ~Compositor();

    Compositor(const Compositor&) = delete;
    auto operator=(const Compositor&) -> Compositor& = delete;

    Compositor(Compositor&&) = delete;
    auto operator=(Compositor&&) -> Compositor& = delete;

    auto set_wl_compositor(wl_compositor* compositor) -> void;

    [[nodiscard]] auto get_wl_compositor() const -> wl_compositor*;

   protected:
   private:
    wl_compositor* m_compositor{};
};
}  // namespace wall
