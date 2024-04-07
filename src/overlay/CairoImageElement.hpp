#pragma once

#include <cairo/cairo.h>
#include <gdk-pixbuf/gdk-pixbuf-core.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <filesystem>
#include "conf/Config.hpp"

namespace wall {
class CairoImageElement {
   public:
    explicit CairoImageElement(const Config& config);

    virtual ~CairoImageElement();

    auto draw(cairo_t* cairo, double center_x, double center_y, double radius) const -> void;

    auto update_settings() -> void;

   protected:
    auto premultiply_alpha(const guint8* gdkpix, cairo_surface_t* cairo_surface, int width, int height, int stride) const -> void;

    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto load_image(const std::filesystem::path& file_path) const -> cairo_surface_t*;

    [[nodiscard]] auto buffer_to_surface(GdkPixbuf* gdkbuf) const -> cairo_surface_t*;

   private:
    const Config& m_config;

    std::filesystem::path m_image_path;

    cairo_surface_t* m_image{};

    double m_image_width{};
    double m_image_height{};
};
}  // namespace wall
