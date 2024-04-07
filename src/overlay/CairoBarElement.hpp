#pragma once

#include <cairo.h>

#include "conf/Config.hpp"
#include "overlay/CairoFontCache.hpp"
#include "overlay/Color.hpp"

namespace wall {

enum class BarAlignment {
    None,
    TopLeft,
    TopCenter,
    TopRight,
    BottomRight,
    BottomCenter,
    BottomLeft,
};

class CairoBarElement {
   public:
    CairoBarElement(const Config& config);
    ~CairoBarElement() = default;

    auto update_settings() -> void;

    auto draw(cairo_t* cairo, double width, double height, const CairoFontCache& font_cache, const std::string& message) const -> void;

    [[nodiscard]] auto get_position_size(int32_t width, int32_t height, int32_t buffer_width, int32_t buffer_height) const
        -> std::pair<int32_t, int32_t>;

    [[nodiscard]] auto get_buffer_size(int32_t width,
                                       int32_t height,
                                       int32_t scale,
                                       const CairoFontCache& font_cache,
                                       const std::string& message) const -> std::pair<int32_t, int32_t>;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto alignment_from_string(std::string_view str_orig) const -> BarAlignment;

    auto draw_text(cairo_t* cairo, double width, double height, const CairoFontCache& font_cache, const std::string& message) const -> void;

    auto draw_background(cairo_t* cairo, double width, double height) const -> void;

    auto rounded_rectangle(cairo_t* cairo, double x_pos, double y_pos, double width, double height, double corner_radius_1) const -> void;

   private:
    const Config& m_config;

    BarAlignment m_alignment{BarAlignment::None};

    Color m_font_color{};
    Color m_background_color{};
    Color m_border_color{};

    int32_t m_border_width{};
    int32_t m_corner_radius{};

    int32_t m_left_padding{};
    int32_t m_right_padding{};
    int32_t m_top_padding{};
    int32_t m_bottom_padding{};
    int32_t m_text_top_bottom_margin{};
};
}  // namespace wall
