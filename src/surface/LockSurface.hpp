#pragma once

#include <wayland-client-protocol.h>
#include "display/Screen.hpp"
#include "ext-session-lock-v1-protocol.h"
#include "surface/Surface.hpp"

namespace wall {

class MpvResource;
class Registry;
class Display;

class LockSurface : public Surface {
   public:
    LockSurface(const Config& config, std::string output_name, Display* display, Registry* registry, wl_output* output);
    ~LockSurface() override;

    LockSurface(const LockSurface&) = delete;
    auto operator=(const LockSurface&) -> LockSurface& = delete;
    LockSurface(LockSurface&&) = delete;
    auto operator=(LockSurface&&) -> LockSurface& = delete;

    auto create(ext_session_lock_v1* lock) -> void;

    auto on_configure(uint32_t serial, uint32_t width, uint32_t height) -> void override;

    [[nodiscard]] auto get_resource_mode() const -> ResourceMode override;

    [[nodiscard]] auto get_lock_surface() const -> ext_session_lock_surface_v1*;

    auto draw_overlay() -> void override;

    auto destroy_resources() -> void override;

   protected:
   private:
    static const ext_session_lock_surface_v1_listener k_listener;

    ext_session_lock_surface_v1* m_lock_surface{};
};
}  // namespace wall
