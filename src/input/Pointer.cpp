#include "input/Pointer.hpp"

#include <spdlog/common.h>
#include <wayland-client-protocol.h>

#include "util/Log.hpp"

struct wl_pointer;
struct wl_surface;

#pragma GCC diagnostic push

// Ignore the warning about missing field initializers in the struct, some older versions of the protocol are missing .axis_value120

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

const wl_pointer_listener wall::Pointer::k_listener = {
    .enter =
        [](void* /* data */, wl_pointer* /* pointer */, uint32_t /* serial */, wl_surface* /* surface */, wl_fixed_t /* x */, wl_fixed_t /* y */) {
            LOG_DEBUG("Pointer enter");
        },
    .leave = [](void* /* data */, wl_pointer* /* pointer */, uint32_t /* serial */, wl_surface* /* surface */) { LOG_DEBUG("Pointer leave"); },
    .motion =
        [](void* /* data */, wl_pointer* /* pointer */, uint32_t /* time */, wl_fixed_t /* x */, wl_fixed_t /* y */) { LOG_DEBUG("Pointer motion"); },
    .button =
        [](void* /* data */, wl_pointer* /* pointer */, uint32_t /* serial */, uint32_t /* time */, uint32_t /* button */, uint32_t /* state */) {
            LOG_DEBUG("Pointer button");
        },
    .axis =
        [](void* /* data */, wl_pointer* /* pointer */, uint32_t /* time */, uint32_t /* axis */, wl_fixed_t /* value */) {
            LOG_DEBUG("Pointer axis");
        },
    .frame = [](void* /* data */, wl_pointer* /* pointer */) { LOG_DEBUG("Pointer frame"); },
};

wall::Pointer::Pointer(wl_pointer* pointer) : m_pointer{pointer} {
    if (m_pointer == nullptr) {
        return;
    }

    wl_pointer_add_listener(m_pointer, &k_listener, this);
}

wall::Pointer::~Pointer() {
    if (m_pointer != nullptr) {
        wl_pointer_destroy(m_pointer);
    }
}

auto wall::Pointer::set_wl_pointer(wl_pointer* pointer) -> void {
    if (m_pointer != nullptr) {
        wl_pointer_destroy(m_pointer);
    }

    m_pointer = pointer;
    wl_pointer_add_listener(m_pointer, &k_listener, this);
}

auto wall::Pointer::on_enter([[maybe_unused]] uint32_t serial,
                             [[maybe_unused]] wl_surface* surface,
                             [[maybe_unused]] wl_fixed_t x_coor,
                             [[maybe_unused]] wl_fixed_t y_coor) -> void {
    LOG_DEBUG("Pointer enter");
}

auto wall::Pointer::on_leave([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface* surface) -> void { LOG_DEBUG("Pointer leave"); }

auto wall::Pointer::on_motion([[maybe_unused]] uint32_t time, [[maybe_unused]] wl_fixed_t x_coor, [[maybe_unused]] wl_fixed_t y_coor) -> void {
    LOG_DEBUG("Pointer motion");
}

auto wall::Pointer::on_button([[maybe_unused]] uint32_t serial,
                              [[maybe_unused]] uint32_t time,
                              [[maybe_unused]] uint32_t button,
                              [[maybe_unused]] uint32_t state) -> void {
    LOG_DEBUG("Pointer button");
}

auto wall::Pointer::on_axis([[maybe_unused]] uint32_t time, [[maybe_unused]] uint32_t axis, [[maybe_unused]] wl_fixed_t value) -> void {
    LOG_DEBUG("Pointer axis");
}

auto wall::Pointer::on_frame() -> void { LOG_DEBUG("Pointer frame"); }

auto wall::Pointer::on_axis_source([[maybe_unused]] uint32_t axis_source) -> void { LOG_DEBUG("Pointer axis source"); }

auto wall::Pointer::on_axis_stop([[maybe_unused]] uint32_t time, [[maybe_unused]] uint32_t axis) -> void { LOG_DEBUG("Pointer axis stop"); }

auto wall::Pointer::on_axis_discrete([[maybe_unused]] uint32_t axis, [[maybe_unused]] int32_t discrete) -> void {
    LOG_DEBUG("Pointer axis discrete");
}

auto wall::Pointer::on_axis_value120([[maybe_unused]] uint32_t axis, [[maybe_unused]] wl_fixed_t value) -> void {
    LOG_DEBUG("Pointer axis value120");
}

auto wall::Pointer::on_axis_relative_direction([[maybe_unused]] uint32_t axis, [[maybe_unused]] uint32_t direction) -> void {
    LOG_DEBUG("Pointer axis relative direction");
}

#pragma GCC diagnostic pop
