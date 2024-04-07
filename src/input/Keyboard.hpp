#pragma once

#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>
#include <chrono>
#include <functional>
#include <string>
#include "util/Loop.hpp"

namespace wall {

class Keyboard;
struct OnKeyState {
    xkb_keysym_t m_keysym{};
    std::string m_utf8_key{};
    Keyboard* m_keyboard{};
    void* m_data{};
};

using on_key_callback = std::function<void(const OnKeyState&)>;

class Keyboard {
   public:
    explicit Keyboard(Loop* loop, wl_keyboard* keyboard);
    virtual ~Keyboard();

    Keyboard(const Keyboard&) = delete;
    auto operator=(const Keyboard&) -> Keyboard& = delete;

    Keyboard(Keyboard&&) = default;
    auto operator=(Keyboard&&) -> Keyboard& = default;

    auto set_wl_keyboard(wl_keyboard* keyboard) -> void;

    [[nodiscard]] auto add_on_key_callback(const on_key_callback& callback, void* data) -> uint32_t;

    auto remove_on_key_callback(uint32_t callback_id) -> void;

    [[nodiscard]] virtual auto is_caps_lock() const -> bool { return m_is_caps_lock; }

    [[nodiscard]] virtual auto is_ctrl() const -> bool { return m_is_ctrl; }

    auto stop_repeat() -> void;

    [[nodiscard]] virtual auto get_layout() const -> const std::string&;

   protected:
    auto on_keymap(uint32_t format, int file, uint32_t size) -> void;

    auto on_enter(uint32_t serial, wl_surface* surface, const wl_array* keys) -> void;

    auto on_leave(uint32_t serial, wl_surface* surface) -> void;

    auto on_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) -> void;

    auto on_modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) -> void;

    auto on_repeat_info(int32_t rate, int32_t delay) -> void;

    auto on_repeat() -> void;

    auto update_layout() -> void;

    auto update_caps_lock() -> void;

    auto fire_on_key_callbacks(xkb_keysym_t keysym, const std::string& utf8_key) -> void;

    [[nodiscard]] auto get_utf8_key(uint32_t keycode) const -> std::string;

   private:
    Loop* m_loop{};

    std::vector<std::pair<on_key_callback, void*>> m_on_key_callbacks;

    const static wl_keyboard_listener k_listener;

    wl_keyboard* m_keyboard{};

    struct xkb_context* m_xkb_context{};

    struct xkb_state* m_xkb_state{};

    xkb_keymap* m_keymap{};

    bool m_is_caps_lock{false};

    bool m_is_ctrl{false};

    loop::Timer* m_repeat_timer{};

    uint32_t m_repeat_sym{};

    std::string m_repeat_utf8_key{};

    std::string m_current_layout{};

    int32_t m_current_layout_index{};

    std::chrono::milliseconds m_repeat_period{};

    std::chrono::milliseconds m_repeat_delay{};
};
}  // namespace wall
