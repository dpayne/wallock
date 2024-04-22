#pragma once

#include <cairo.h>
#include <wayland-client-protocol.h>
#include "State.hpp"
#include "conf/Config.hpp"
#include "overlay/CairoAnalogClockElement.hpp"
#include "overlay/CairoIndicatorElement.hpp"
#include "overlay/CairoIndicatorMessage.hpp"
#include "overlay/CairoSurface.hpp"

namespace wall {
class Surface;

class CairoIndicatorSurface : public CairoSurface {
   public:
    CairoIndicatorSurface(const Config& config, Surface* surface, wl_output_subpixel subpixel);
    ~CairoIndicatorSurface() override;

    auto update_settings() -> void;

    auto on_state_change(State state) -> void override;

   protected:
    auto draw_frame(int32_t width, int32_t height) -> std::chrono::milliseconds override;

    virtual auto get_buffer_size(int32_t buffer_diameter) -> std::pair<int32_t, int32_t>;

    auto resize_based_on_text(int32_t* width) -> void;

    auto draw_message(cairo_t* cairo, double radius, double width, double height) -> void;

    auto update_message() -> void;

    auto should_draw() -> bool;

    auto get_indicator_message() -> const CairoIndicatorMessage&;

   private:
    struct StateCheck {
        int32_t m_width{};
        int32_t m_height{};
        double m_ring_highlight_start{};
        State m_indicator_state{};
        std::string m_message{};

        auto operator==(const StateCheck& other) const -> bool {
            return m_width == other.m_width && m_height == other.m_height && m_message == other.m_message &&
                   m_indicator_state == other.m_indicator_state && m_ring_highlight_start == other.m_ring_highlight_start;
        }
    };

    bool m_is_lock_enabled{};
    bool m_is_wallpaper_enabled{};
    bool m_is_analog_clock_enabled{};
    bool m_is_primary_only{};

    CairoIndicatorElement m_indicator;
    CairoIndicatorMessage m_indicator_message;
    CairoAnalogClockElement m_analog_clock;

    struct StateCheck m_last_state {};
};
}  // namespace wall
