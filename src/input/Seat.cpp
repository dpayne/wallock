#include "input/Seat.hpp"
#include "display/Display.hpp"

#include <wayland-client-protocol.h>

#include "util/Log.hpp"

const wl_seat_listener wall::Seat::k_listener = {
    .capabilities =
        [](void* data, wl_seat* /* seat */, uint32_t capabilities) {
            auto& self = *static_cast<wall::Seat*>(data);
            self.on_capabilities(capabilities);
        },
    .name =
        [](void* data, wl_seat* /* seat */, const char* name) {
            auto& self = *static_cast<wall::Seat*>(data);
            self.on_name(name);
        },
};

wall::Seat::Seat(Loop* loop, wl_seat* seat)
    : m_loop{loop}, m_seat{seat}, m_pointer{std::make_unique<wall::Pointer>(nullptr)}, m_keyboard{std::make_unique<wall::Keyboard>(loop, nullptr)} {
    if (m_seat == nullptr) {
        return;
    }

    wl_seat_add_listener(m_seat, &k_listener, this);
}

wall::Seat::~Seat() {
    if (m_seat != nullptr) {
        wl_seat_destroy(m_seat);
    }
}

auto wall::Seat::get_wl_seat() const -> wl_seat* { return m_seat; }

auto wall::Seat::set_wl_seat(wl_seat* seat) -> void {
    if (m_seat != nullptr) {
        wl_seat_destroy(m_seat);
    }

    m_seat = seat;
    wl_seat_add_listener(m_seat, &k_listener, this);
}

auto wall::Seat::on_capabilities([[maybe_unused]] uint32_t capabilities) -> void {
    m_pointer->set_wl_pointer(wl_seat_get_pointer(m_seat));
    m_keyboard->set_wl_keyboard(wl_seat_get_keyboard(m_seat));
}

auto wall::Seat::on_name(const char* name) -> void {
    if (name == nullptr) {
        return;
    }

    LOG_DEBUG("Name: {}", name);
    m_name = name;
}
