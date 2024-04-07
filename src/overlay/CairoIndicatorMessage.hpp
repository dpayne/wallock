#pragma once

#include <cairo.h>
#include "State.hpp"
#include "conf/Config.hpp"
#include "overlay/CairoFontCache.hpp"
#include "overlay/ColorSet.hpp"

namespace wall {
class CairoIndicatorMessage {
   public:
    CairoIndicatorMessage(const Config& config);
    ~CairoIndicatorMessage() = default;

    auto update_settings() -> void;

    auto update_message(State state, std::chrono::time_point<std::chrono::system_clock> now) -> void;

    [[nodiscard]] auto get_text_width(const CairoFontCache& font_cache) const -> double;

    auto draw(cairo_t* cairo, State state, const CairoFontCache& font_cache, double x_pos, double y_pos) -> std::chrono::milliseconds;

    [[nodiscard]] auto get_message() const -> const std::string&;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto get_message_format(State state) const -> const std::string&;

   private:
    const Config& m_config;

    bool m_is_clock_enabled{};

    std::string m_message{};
    std::string m_message_input{};
    std::string m_message_cleared{};
    std::string m_message_caps_lock{};
    std::string m_message_verifying{};
    std::string m_message_wrong{};

    ColorSet m_font_color{};
};
}  // namespace wall
