#include "overlay/CairoImageElement.hpp"

#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf-core.h>
#include <glib-object.h>
#include <spdlog/common.h>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>

#include "conf/ConfigMacros.hpp"
#include "util/FileUtils.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
}  // namespace wall

/**
 * Much of this code was taken from swaylock-effects
 */
wall::CairoImageElement::CairoImageElement(const Config& config) : m_config(config) { update_settings(); }

wall::CairoImageElement::~CairoImageElement() {
    if (m_image != nullptr) {
        cairo_surface_destroy(m_image);
    }
}

auto wall::CairoImageElement::update_settings() -> void {
    m_image_path = FileUtils::expand_path(std::string{wall_conf_get(get_config(), lock_indicator, image_path)}).value_or("");
    if (m_image != nullptr) {
        cairo_surface_destroy(m_image);
    }

    if (!m_image_path.empty()) {
        m_image = load_image(m_image_path);
        m_image_height = cairo_image_surface_get_height(m_image);
        m_image_width = cairo_image_surface_get_width(m_image);
    }
}

[[nodiscard]] auto wall::CairoImageElement::get_config() const -> const Config& { return m_config; }

auto wall::CairoImageElement::load_image(const std::filesystem::path& file_path) const -> cairo_surface_t* {
    GError* err = nullptr;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(file_path.c_str(), &err);
    if (pixbuf == nullptr) {
        LOG_ERROR("Failed to load image ({}): {}", file_path.string(), err->message);
    }
    auto* image = buffer_to_surface(pixbuf);
    g_object_unref(pixbuf);
    return image;
}

auto wall::CairoImageElement::buffer_to_surface(GdkPixbuf* gdkbuf) const -> cairo_surface_t* {
    const auto chan = gdk_pixbuf_get_n_channels(gdkbuf);
    if (chan < 3) {
        return nullptr;
    }

    const guint8* gdkpix = gdk_pixbuf_read_pixels(gdkbuf);
    if (gdkpix == nullptr) {
        return nullptr;
    }
    const auto width = gdk_pixbuf_get_width(gdkbuf);
    const auto height = gdk_pixbuf_get_height(gdkbuf);
    int stride = gdk_pixbuf_get_rowstride(gdkbuf);

    cairo_format_t fmt = (chan == 3) ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32;
    cairo_surface_t* cairo_surface = cairo_image_surface_create(fmt, width, height);
    cairo_surface_flush(cairo_surface);
    if (cairo_surface == nullptr || cairo_surface_status(cairo_surface) != CAIRO_STATUS_SUCCESS) {
        return nullptr;
    }

    int cstride = cairo_image_surface_get_stride(cairo_surface);
    unsigned char* cpix = cairo_image_surface_get_data(cairo_surface);

    if (chan == 3) {
        int32_t height_ix{};
        for (height_ix = height; height_ix > 0; --height_ix) {
            const guint8* gdk_buffer = gdkpix;
            unsigned char* cairo_pixel_buffer = cpix;
            const guint8* end = gdk_buffer + 3 * width;
            while (gdk_buffer < end) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
                cairo_pixel_buffer[0] = gdk_buffer[2];
                cairo_pixel_buffer[1] = gdk_buffer[1];
                cairo_pixel_buffer[2] = gdk_buffer[0];
#else
                cp[1] = gp[0];
                cp[2] = gp[1];
                cp[3] = gp[2];
#endif
                gdk_buffer += 3;
                cairo_pixel_buffer += 4;
            }
            gdkpix += stride;
            cpix += cstride;
        }
    } else {
        premultiply_alpha(gdkpix, cairo_surface, width, height, stride);
    }
    cairo_surface_mark_dirty(cairo_surface);
    return cairo_surface;
}

auto wall::CairoImageElement::premultiply_alpha(const guint8* gdkpix,
                                                cairo_surface_t* cairo_surface,
                                                int32_t width,
                                                int32_t height,
                                                int32_t stride) const -> void {
    auto* cairo_buffer = cairo_image_surface_get_data(cairo_surface);
    int cstride = cairo_image_surface_get_stride(cairo_surface);

    /* premul-color = alpha/255 * color/255 * 255 = (alpha*color)/255
     * (z/255) = z/256 * 256/255     = z/256 (1 + 1/255)
     *         = z/256 + (z/256)/255 = (z + z/255)/256
     *         # recurse once
     *         = (z + (z + z/255)/256)/256
     *         = (z + z/256 + z/256/255) / 256
     *         # only use 16bit uint operations, loose some precision,
     *         # result is floored.
     *       ->  (z + z>>8)>>8
     *         # add 0x80/255 = 0.5 to convert floor to round
     *       =>  (z+0x80 + (z+0x80)>>8 ) >> 8
     * ------
     * tested as equal to lround(z/255.0) for uint z in [0..0xfe02]
     */
#define PREMUL_ALPHA(x, a, b, z) \
    G_STMT_START {               \
        z = a * b + 0x80;        \
        x = (z + (z >> 8)) >> 8; \
    }                            \
    G_STMT_END
    auto height_ix = 0UL;
    for (height_ix = height; height_ix > 0; --height_ix) {
        const guint8* inner_gdk_buffer = gdkpix;
        unsigned char* inner_cairo_buffer = cairo_buffer;
        const guint8* end = inner_gdk_buffer + 4 * width;
        uint32_t z_pix_1{};
        uint32_t z_pix_2{};
        uint32_t z_pix_3{};
        while (inner_gdk_buffer < end) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
            PREMUL_ALPHA(inner_cairo_buffer[0], inner_gdk_buffer[2], inner_gdk_buffer[3], z_pix_1);
            PREMUL_ALPHA(inner_cairo_buffer[1], inner_gdk_buffer[1], inner_gdk_buffer[3], z_pix_2);
            PREMUL_ALPHA(inner_cairo_buffer[2], inner_gdk_buffer[0], inner_gdk_buffer[3], z_pix_3);
            inner_cairo_buffer[3] = inner_gdk_buffer[3];
#else
            PREMUL_ALPHA(cp[1], gp[0], gp[3], z1);
            PREMUL_ALPHA(cp[2], gp[1], gp[3], z2);
            PREMUL_ALPHA(cp[3], gp[2], gp[3], z3);
            cp[0] = gp[3];
#endif
            inner_gdk_buffer += 4;
            inner_cairo_buffer += 4;
        }
        gdkpix += stride;
        cairo_buffer += cstride;
    }
#undef PREMUL_ALPHA
}

auto wall::CairoImageElement::draw(cairo_t* cairo, double center_x, double center_y, double radius) const -> void {
    if (m_image == nullptr) {
        return;
    }

    const auto smallest = std::min(m_image_height, m_image_width);
    const auto scale = (radius * 2.0) / smallest;
    const auto offset = (radius / scale) - (smallest * 0.5);

    // Create the arc that clips the image
    cairo_arc(cairo, center_x, center_y, radius, 0, 2 * M_PI);

    // Scale cairo to make image fit the indicator
    cairo_scale(cairo, scale, scale);
    cairo_set_source_surface(cairo, m_image, offset, offset);
    // Scale cairo back
    cairo_scale(cairo, 1.0 / scale, 1.0 / scale);
    cairo_fill(cairo);
}
