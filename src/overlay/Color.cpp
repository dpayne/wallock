#include "overlay/Color.hpp"

#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>
#include <string>
#include <string_view>

#include "util/Log.hpp"
#include "util/StringUtils.hpp"

wall::Color::Color(std::string_view color) {
    const auto color_from_str = from_string(color);
    m_red = color_from_str.m_red;
    m_green = color_from_str.m_green;
    m_blue = color_from_str.m_blue;
    m_alpha = color_from_str.m_alpha;
}

wall::Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) : m_red{red}, m_green{green}, m_blue{blue}, m_alpha{alpha} {}

auto wall::Color::from_hex(uint32_t hex) -> Color {
    return Color{static_cast<uint8_t>((hex >> 24) & 0xFF), static_cast<uint8_t>((hex >> 16) & 0xFF), static_cast<uint8_t>((hex >> 8) & 0xFF),
                 static_cast<uint8_t>(hex & 0xFF)};
}

auto wall::Color::set_cairo_color(cairo_t* cairo, const Color& color) -> void {
    cairo_set_source_rgba(cairo, static_cast<double>(color.m_red) / 255.0, static_cast<double>(color.m_green) / 255.0,
                          static_cast<double>(color.m_blue) / 255.0, static_cast<double>(color.m_alpha) / 255.0);
}

auto wall::Color::from_string(std::string_view color) -> Color {
    auto color_trimmed = StringUtils::trim(color);
    color_trimmed = StringUtils::to_upper(color_trimmed);
    if (color_trimmed.size() == 7 && color_trimmed[0] == '#') {
        color_trimmed += "FF";
    }

    if (color_trimmed.size() == 9 && color_trimmed[0] == '#') {
        return from_hex(std::stoul(color_trimmed.substr(1), nullptr, 16));
    }

    LOG_ERROR("Invalid color string: {}", color);
    return Color{};
}

auto wall::Color::to_string() const -> std::string { return fmt::format("#{:02X}{:02X}{:02X}{:02X}", m_red, m_green, m_blue, m_alpha); }
