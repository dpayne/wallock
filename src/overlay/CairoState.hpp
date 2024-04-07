#pragma once

#include <cairo.h>
#include <cstdint>
#include <memory>

#include "registry/BufferPool.hpp"

namespace wall {
class CairoState {
   public:
    CairoState(int32_t width, int32_t height, int32_t pixel_size, cairo_t* cairo, cairo_surface_t* surface, std::unique_ptr<Buffer> buffer);

    ~CairoState();

    [[nodiscard]] auto get_cairo() const -> cairo_t*;

    [[nodiscard]] auto get_surface() const -> cairo_surface_t*;

    [[nodiscard]] auto get_buffer() const -> Buffer*;

    [[nodiscard]] auto get_width() const -> int32_t;

    [[nodiscard]] auto get_height() const -> int32_t;

    [[nodiscard]] auto get_pixel_width() const -> int32_t;

   protected:
   private:
    int32_t m_width{};
    int32_t m_height{};
    int32_t m_pixel_width{};

    cairo_t* m_cairo{};

    cairo_surface_t* m_surface{};

    std::unique_ptr<Buffer> m_buffer{};
};
}  // namespace wall
