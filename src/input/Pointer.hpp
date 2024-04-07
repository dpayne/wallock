#pragma once

#include <wayland-client-protocol.h>

namespace wall {
class Pointer {
   public:
    explicit Pointer(wl_pointer* pointer);
    virtual ~Pointer();

    Pointer(const Pointer&) = delete;
    auto operator=(const Pointer&) -> Pointer& = delete;

    Pointer(Pointer&&) = default;
    auto operator=(Pointer&&) -> Pointer& = default;

    auto set_wl_pointer(wl_pointer* pointer) -> void;

   protected:
    auto on_enter(uint32_t serial, wl_surface* surface, wl_fixed_t x_coor, wl_fixed_t y_coor) -> void;

    auto on_leave(uint32_t serial, wl_surface* surface) -> void;

    auto on_motion(uint32_t time, wl_fixed_t x_coor, wl_fixed_t y_coor) -> void;

    auto on_button(uint32_t serial, uint32_t time, uint32_t button, uint32_t state) -> void;

    auto on_axis(uint32_t time, uint32_t axis, wl_fixed_t value) -> void;

    auto on_frame() -> void;

    auto on_axis_source(uint32_t axis_source) -> void;

    auto on_axis_stop(uint32_t time, uint32_t axis) -> void;

    auto on_axis_discrete(uint32_t axis, int32_t discrete) -> void;

    auto on_axis_value120(uint32_t axis, wl_fixed_t value) -> void;

    auto on_axis_relative_direction(uint32_t axis, uint32_t direction) -> void;

   private:
    static const wl_pointer_listener k_listener;

    wl_pointer* m_pointer{};
};
}  // namespace wall
