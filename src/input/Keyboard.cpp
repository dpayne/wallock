/**
 * Much of this logic was taken from swalock's seat.c file then modified to fit the needs of this project.
 */
#include "input/Keyboard.hpp"

#include <spdlog/common.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon-names.h>

#include "util/Log.hpp"

struct wl_array;
struct wl_keyboard;
struct wl_surface;

const wl_keyboard_listener wall::Keyboard::k_listener = {
    .keymap =
        [](void* data, wl_keyboard* /* keyboard */, uint32_t format, int file, uint32_t size) {
            static_cast<Keyboard*>(data)->on_keymap(format, file, size);
        },
    .enter =
        [](void* data, wl_keyboard* /* keyboard */, uint32_t serial, wl_surface* surface, wl_array* keys) {
            static_cast<Keyboard*>(data)->on_enter(serial, surface, keys);
        },
    .leave =
        [](void* data, wl_keyboard* /* keyboard */, uint32_t serial, wl_surface* surface) {
            static_cast<Keyboard*>(data)->on_leave(serial, surface);
        },
    .key =
        [](void* data, wl_keyboard* /* keyboard */, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
            static_cast<Keyboard*>(data)->on_key(serial, time, key, state);
        },
    .modifiers = [](void* data,
                    wl_keyboard* /* keyboard */,
                    uint32_t serial,
                    uint32_t mods_depressed,
                    uint32_t mods_latched,
                    uint32_t mods_locked,
                    uint32_t group) { static_cast<Keyboard*>(data)->on_modifiers(serial, mods_depressed, mods_latched, mods_locked, group); },
    .repeat_info =
        [](void* data, wl_keyboard* /* keyboard */, int32_t rate, int32_t delay) { static_cast<Keyboard*>(data)->on_repeat_info(rate, delay); },
};

wall::Keyboard::Keyboard(Loop* loop, wl_keyboard* keyboard)
    : m_loop{loop}, m_keyboard{keyboard}, m_xkb_context{xkb_context_new(XKB_CONTEXT_NO_FLAGS)} {
    if (m_keyboard == nullptr) {
        return;
    }

    wl_keyboard_add_listener(m_keyboard, &k_listener, this);
}

wall::Keyboard::~Keyboard() {
    if (m_keyboard != nullptr) {
        wl_keyboard_destroy(m_keyboard);
    }

    if (m_keymap != nullptr) {
        xkb_keymap_unref(m_keymap);
    }

    if (m_xkb_state != nullptr) {
        xkb_state_unref(m_xkb_state);
    }

    if (m_xkb_context != nullptr) {
        xkb_context_unref(m_xkb_context);
    }

    stop_repeat();
}

auto wall::Keyboard::set_wl_keyboard(wl_keyboard* keyboard) -> void {
    if (m_keyboard != nullptr) {
        wl_keyboard_destroy(m_keyboard);
    }

    m_keyboard = keyboard;
    wl_keyboard_add_listener(m_keyboard, &k_listener, this);
}

auto wall::Keyboard::on_keymap(uint32_t format, int file, uint32_t size) -> void {
    LOG_DEBUG("Keyboard keymap {} {}", file, size);

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(file);
        LOG_FATAL("Unsupported keymap format");
    }
    auto* map_shm = mmap(nullptr, size - 1, PROT_READ, MAP_PRIVATE, file, 0);
    if (map_shm == MAP_FAILED) {  // NOLINT
        close(file);
        LOG_FATAL("Failed to mmap keymap");
    }

    auto* keymap =
        xkb_keymap_new_from_buffer(m_xkb_context, (const char*)map_shm, size - 1, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);  // NOLINT
    munmap(map_shm, size - 1);
    close(file);
    auto* xkb_state = xkb_state_new(keymap);
    xkb_keymap_unref(m_keymap);
    xkb_state_unref(m_xkb_state);
    m_keymap = keymap;
    m_xkb_state = xkb_state;

    update_layout();
}

auto wall::Keyboard::on_enter([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface* surface, [[maybe_unused]] const wl_array* keys) -> void {
    LOG_DEBUG("Keyboard enter");
}

auto wall::Keyboard::on_leave([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface* surface) -> void { stop_repeat(); }

auto wall::Keyboard::on_key(uint32_t /* serial */, uint32_t /* time */, uint32_t key, uint32_t state) -> void {
    if (m_keymap == nullptr || m_xkb_state == nullptr) {
        return;
    }

    LOG_DEBUG("Keyboard key {} {}", key, state);
    const auto key_state = static_cast<wl_keyboard_key_state>(state);

    // wayland keycodes are offset by 8
    const auto sym = xkb_state_key_get_one_sym(m_xkb_state, key + 8);
    const auto utf8_key = get_utf8_key(key + 8);
    stop_repeat();

    if (key_state == WL_KEYBOARD_KEY_STATE_PRESSED && m_repeat_period.count() > 0) {
        m_repeat_sym = sym;
        m_repeat_utf8_key = utf8_key;
        m_repeat_timer = m_loop->add_timer(m_repeat_delay, m_repeat_period, [this](loop::Timer*) { this->on_repeat(); });
    }

    // Must be last, since it could trigger an unlock and might delete the keyboard object
    if (key_state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        fire_on_key_callbacks(sym, utf8_key);
    }
}

auto wall::Keyboard::get_utf8_key(uint32_t keycode) const -> std::string {
    char* buffer{};
    int32_t size{};
    size = xkb_state_key_get_utf8(m_xkb_state, keycode, nullptr, 0) + 1;
    if (size > 1) {
        buffer = new char[size];
        xkb_state_key_get_utf8(m_xkb_state, keycode, buffer, size);

        // Remove null terminator
        std::string result = std::string(buffer, size - 1);
        delete[] buffer;
        return result;
    }

    return "";
}

auto wall::Keyboard::add_on_key_callback(const on_key_callback& callback, void* data) -> uint32_t {
    m_on_key_callbacks.emplace_back(callback, data);
    return m_on_key_callbacks.size();
}

auto wall::Keyboard::remove_on_key_callback(uint32_t callback_id) -> void {
    if (callback_id > 0 && callback_id > m_on_key_callbacks.size()) {
        return;
    }

    m_on_key_callbacks.erase(m_on_key_callbacks.begin() + (callback_id - 1));
}

auto wall::Keyboard::fire_on_key_callbacks(xkb_keysym_t keysym, const std::string& utf8_key) -> void {
    for (const auto& callback : m_on_key_callbacks) {
        callback.first({keysym, utf8_key, this, callback.second});
    }
}

auto wall::Keyboard::on_modifiers(uint32_t /* serial */, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
    -> void {
    if (m_xkb_state == nullptr) {
        return;
    }

    const auto layout_same = xkb_state_layout_index_is_active(m_xkb_state, group, XKB_STATE_LAYOUT_EFFECTIVE);
    if (layout_same != m_current_layout_index) {
        update_layout();
        fire_on_key_callbacks(0, "");
    }

    xkb_state_update_mask(m_xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
    const auto is_caps_lock = xkb_state_mod_name_is_active(m_xkb_state, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_LOCKED) == 1;
    if (is_caps_lock != m_is_caps_lock) {
        LOG_DEBUG("Caps lock changed")
        m_is_caps_lock = is_caps_lock;
        fire_on_key_callbacks(0, "");
    }

    m_is_ctrl = (xkb_state_mod_name_is_active(m_xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_DEPRESSED) == 1) ||
                (xkb_state_mod_name_is_active(m_xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_LATCHED) == 1);
}

auto wall::Keyboard::on_repeat_info(int32_t rate, int32_t delay) -> void {
    LOG_DEBUG("Keyboard repeat info");
    if (rate <= 0) {
        m_repeat_period = std::chrono::milliseconds(-1);
    } else {
        // Keys per second -> milliseconds between keys
        m_repeat_period = std::chrono::milliseconds(1000 / rate);  // NOLINT
    }

    m_repeat_delay = std::chrono::milliseconds(delay);
}

auto wall::Keyboard::stop_repeat() -> void {
    if (m_repeat_timer == nullptr) {
        return;
    }

    m_repeat_timer->close();
    m_repeat_timer = nullptr;
}

auto wall::Keyboard::on_repeat() -> void {
    LOG_DEBUG("Keyboard repeat timer event");
    fire_on_key_callbacks(m_repeat_sym, m_repeat_utf8_key);
}

auto wall::Keyboard::get_layout() const -> const std::string& { return m_current_layout; }

auto wall::Keyboard::update_layout() -> void {
    if (m_keyboard == nullptr) {
        return;
    }

    xkb_layout_index_t num_layout = xkb_keymap_num_layouts(m_keymap);
    if (num_layout > 0) {
        xkb_layout_index_t curr_layout = 0;

        // advance to the first active layout (if any)
        while (curr_layout < num_layout && xkb_state_layout_index_is_active(m_xkb_state, curr_layout, XKB_STATE_LAYOUT_EFFECTIVE) != 1) {
            ++curr_layout;
        }

        // will handle invalid index if none are active
        m_current_layout = xkb_keymap_layout_get_name(m_keymap, curr_layout);
        m_current_layout_index = curr_layout;
    }
}
