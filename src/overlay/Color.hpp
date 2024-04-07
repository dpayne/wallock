#ifndef WALL_COLOR_HPP
#define WALL_COLOR_HPP

#include <cairo.h>
#include <cstdint>
#include <string>
#include <string_view>

namespace wall {
struct Color {
    uint8_t m_red{};
    uint8_t m_green{};
    uint8_t m_blue{};
    uint8_t m_alpha{};

    Color() = default;
    Color(std::string_view color);
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

    static auto from_string(std::string_view color) -> Color;

    static auto from_hex(uint32_t hex) -> Color;

    static auto set_cairo_color(cairo_t* cairo, const Color& color) -> void;

    [[nodiscard]] auto to_string() const -> std::string;
};

}  // namespace wall
#endif  // WALL_COLOR_HPP
