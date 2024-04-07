#pragma once

#include <cairo.h>
#include "conf/Config.hpp"
#include "overlay/CairoState.hpp"

namespace wall

{
class CairoFontCache {
   public:
    CairoFontCache(const Config& config, cairo_subpixel_order_t subpixel);
    ~CairoFontCache();

    auto load_font(std::string_view font, double font_size) -> void;

    [[nodiscard]] auto get_font_face() const -> cairo_font_face_t*;

    [[nodiscard]] auto get_font_family() const -> const std::string&;

    [[nodiscard]] auto get_font_size() const -> double;

    [[nodiscard]] auto get_font_cairo_state() const -> CairoState*;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto load_font_face(std::string_view font) const -> cairo_font_face_t*;

    auto configure_font_options(cairo_t* cairo, cairo_font_face_t* font_face, double font_size) const -> void;

   private:
    const Config& m_config;

    cairo_subpixel_order_t m_subpixel;

    std::unique_ptr<CairoState> m_font_cairo_state{};

    std::string m_loaded_font_face;

    cairo_font_face_t* m_font_face{};

    std::string m_font_family{};

    double m_font_size{};
};
}  // namespace wall
