#include "overlay/CairoState.hpp"

#include <utility>

wall::CairoState::CairoState(int32_t width,
                             int32_t height,
                             int32_t pixel_size,
                             cairo_t* cairo,
                             cairo_surface_t* surface,
                             std::unique_ptr<Buffer> buffer)
    : m_width{width}, m_height{height}, m_pixel_width{pixel_size}, m_cairo{cairo}, m_surface{surface}, m_buffer{std::move(buffer)} {}

wall::CairoState::~CairoState() {
    if (m_cairo != nullptr) {
        cairo_destroy(m_cairo);
    }

    if (m_surface != nullptr) {
        cairo_surface_destroy(m_surface);
    }
}

auto wall::CairoState::get_cairo() const -> cairo_t* { return m_cairo; }

auto wall::CairoState::get_surface() const -> cairo_surface_t* { return m_surface; }

auto wall::CairoState::get_buffer() const -> Buffer* { return m_buffer.get(); }

auto wall::CairoState::get_width() const -> int32_t { return m_width; }

auto wall::CairoState::get_height() const -> int32_t { return m_height; }

auto wall::CairoState::get_pixel_width() const -> int32_t { return m_pixel_width; }
