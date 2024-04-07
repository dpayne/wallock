#pragma once

#include <wayland-client.h>
#include <memory>
#include <string>
#include "input/Keyboard.hpp"
#include "input/Pointer.hpp"
#include "util/Loop.hpp"

namespace wall {
class Seat {
   public:
    explicit Seat(Loop* loop, wl_seat* seat);
    virtual ~Seat();

    Seat(const Seat&) = delete;
    auto operator=(const Seat&) -> Seat& = delete;

    Seat(Seat&&) = default;
    auto operator=(Seat&&) -> Seat& = default;

    [[nodiscard]] auto get_pointer() const -> const Pointer& { return *m_pointer; }

    [[nodiscard]] virtual auto get_keyboard() const -> const Keyboard& { return *m_keyboard; }

    [[nodiscard]] auto get_pointer_mut() -> Pointer* { return m_pointer.get(); }

    [[nodiscard]] virtual auto get_keyboard_mut() -> Keyboard* { return m_keyboard.get(); }

    [[nodiscard]] auto get_name() const -> const std::string& { return m_name; }

    auto set_wl_seat(wl_seat* seat) -> void;

    [[nodiscard]] auto get_wl_seat() const -> wl_seat*;

   protected:
    auto on_capabilities(uint32_t capabilities) -> void;

    auto on_name(const char* name) -> void;

   private:
    static const wl_seat_listener k_listener;

    Loop* m_loop{nullptr};

    wl_seat* m_seat{nullptr};

    std::unique_ptr<Pointer> m_pointer{};
    std::unique_ptr<Keyboard> m_keyboard{};

    std::string m_name;
};
}  // namespace wall
