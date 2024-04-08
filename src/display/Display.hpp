#pragma once

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <memory>
#include "conf/Config.hpp"
#include "display/PrimaryDisplayState.hpp"
#include "registry/Lock.hpp"
#include "registry/Registry.hpp"
#include "render/RendererCreator.hpp"
#include "util/Loop.hpp"
#include "util/OnKeyProcessor.hpp"
#include "wlr-input-inhibitor-unstable-v1-protocol.h"

namespace wall {
class Seat;
class SurfaceEGL;
class Surface;
class Screen;

class Display {
   public:
    Display(const Config& config, Loop* loop, bool is_start_locked, std::function<void(void)> on_stop);
    virtual ~Display();

    Display(Display&&) = delete;
    Display(const Display&) = delete;
    auto operator=(const Display&) -> Display = delete;
    auto operator=(Display&&) -> Display = delete;

    auto reload() -> void;

    auto lock() -> void;

    auto stop() -> void;

    auto next() -> void;

    [[nodiscard]] auto is_nvidia() const -> bool;

    [[nodiscard]] auto get_loop() const -> Loop*;

    [[nodiscard]] auto is_locked() const -> bool;

    auto update_settings() -> void;

    [[nodiscard]] auto get_lock_mut() -> Lock*;

    [[nodiscard]] auto get_primary_state_mut() -> PrimaryDisplayState*;

    [[nodiscard]] auto get_renderer_creator_mut() -> RendererCreator*;

    [[nodiscard]] auto get_wl_display() const -> wl_display*;

    auto remove_primary(const std::string& name) -> void;

    auto update_primary(const std::string& name) -> void;

    auto roundtrip(bool is_exit_on_failure = true) const -> void;

    auto flush_events() const -> void;

    auto loop() -> void;

    auto remove_screen(uint32_t global_name) -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto create_pending_surfaces() -> bool;

    auto unlock() -> void;

    auto setup_keyboard_callback() -> void;

    auto on_state_change(State state) -> void;

    auto start_pause_timer() -> void;

    auto stop_pause_timer() -> void;

    auto stop_screens() -> void;

    auto stop_now() -> void;

    [[nodiscard]] auto is_configured() const -> bool;

    auto swap_lock_to_wallpaper() -> void;

    auto swap_wallpaper_to_lock() -> void;

    auto check_for_failure() -> void;

    auto create_lock() -> void;

    static auto detect_nvidia() -> bool;

   private:
    const Config& m_config;

    Loop* m_loop{};

    loop::Poll* m_display_poll{};

    PrimaryDisplayState m_primary_state{};

    std::string m_primary_name_from_config;

    wl_display* m_wl_display{nullptr};

    bool m_is_dispatch_pending{};

    bool m_is_wallpaper_enabled{};

    bool m_is_enforce_input_inhibitor{};

    bool m_is_pause_after_unlock{};

    bool m_is_dismiss_after_pause{};

    loop::Timer* m_pause_timer{};

    std::chrono::seconds m_pause_after_unlock_delay{};

    std::unique_ptr<Registry> m_registry;

    std::unique_ptr<RendererCreator> m_renderer_creator;

    std::unique_ptr<Lock> m_lock;

    bool m_is_locked{};

    uint32_t m_keyboard_callback_id{};

    OnKeyProcessor m_on_key_processor;

    std::string m_password_buffer;

    std::function<void()> m_on_stop;

    std::chrono::milliseconds m_grace_period;

    std::chrono::time_point<std::chrono::system_clock> m_lock_time{};

    struct zwlr_input_inhibitor_v1* m_input_inhibitor{};

    std::vector<std::unique_ptr<Screen>> m_screens_to_be_destroyed;

    bool m_is_shutting_down{false};

    bool m_is_nvidia{false};

    bool m_is_swap_wallpaper_to_lock{};

    bool m_is_swap_lock_to_wallpaper{};
};
}  // namespace wall
