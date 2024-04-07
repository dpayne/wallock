#pragma once

#include <cairo.h>
#include <wayland-client-protocol.h>
#include "conf/Config.hpp"
#include "overlay/CairoBarElement.hpp"
#include "overlay/CairoSurface.hpp"
#include "util/BatteryDiscover.hpp"
#include "util/Formatter.hpp"
#include "util/NetworkDiscover.hpp"

namespace wall {
class Surface;

class CairoBarSurface : public CairoSurface {
   public:
    CairoBarSurface(const Config& config, Surface* surface, wl_output_subpixel subpixel);
    ~CairoBarSurface() override;

    auto update_settings() -> void;

   protected:
    auto draw_frame(int32_t width, int32_t height, int32_t scale) -> std::chrono::milliseconds override;

    [[nodiscard]] virtual auto generate_message() -> std::string;

    [[nodiscard]] auto should_draw() -> bool;

   private:
    struct StateCheck {
        int32_t m_width{};
        int32_t m_height{};
        int32_t m_scale{};
        State m_state{};
        std::string m_message{};

        auto operator==(const StateCheck& other) const -> bool {
            return m_width == other.m_width && m_height == other.m_height && m_scale == other.m_scale && m_message == other.m_message &&
                   m_state == other.m_state;
        }
    };

    static auto alignment_from_string(std::string_view str) -> BarAlignment;

    bool m_is_lock_enabled{};
    bool m_is_wallpaper_enabled{};

    std::string m_keyboard_layout{};

    struct StateCheck m_last_state {};

    CairoBarElement m_bar;

    BatteryDiscover m_battery_discover;
    NetworkDiscover m_network_discover;

    Formatter m_formatter;

    std::vector<Module> m_modules{};

    bool m_is_primary_only{};

    std::string m_module_separator{};

    bool m_is_module_draw_on_empty{};
};
}  // namespace wall
