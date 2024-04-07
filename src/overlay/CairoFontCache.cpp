#include "overlay/CairoFontCache.hpp"

#include <cairo-ft.h>
#include <fontconfig/fontconfig.h>
#include <spdlog/common.h>

#include "util/Log.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::CairoFontCache::CairoFontCache(const Config& config, cairo_subpixel_order_t subpixel) : m_config{config}, m_subpixel{subpixel} {
    auto* text_test_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 1, 1);
    auto* text_test_cairo = cairo_create(text_test_surface);
    cairo_set_antialias(text_test_cairo, CAIRO_ANTIALIAS_BEST);
    m_font_cairo_state = std::make_unique<CairoState>(1, 1, 1, text_test_cairo, text_test_surface, nullptr);
}

wall::CairoFontCache::~CairoFontCache() {
    if (m_font_face != nullptr) {
        cairo_font_face_destroy(m_font_face);
    }
}

auto wall::CairoFontCache::get_font_face() const -> cairo_font_face_t* { return m_font_face; }

auto wall::CairoFontCache::get_font_family() const -> const std::string& { return m_font_family; }

auto wall::CairoFontCache::get_font_size() const -> double { return m_font_size; }

auto wall::CairoFontCache::get_font_cairo_state() const -> CairoState* { return m_font_cairo_state.get(); }

auto wall::CairoFontCache::load_font(std::string_view font, double font_size) -> void {
    if (m_font_face != nullptr && m_loaded_font_face == font && m_font_size == font_size) {
        return;
    }

    m_font_family = font;
    m_font_size = font_size;

    // destroy the old font face
    if (m_font_face != nullptr) {
        cairo_font_face_destroy(m_font_face);
        m_font_face = nullptr;
    }

    auto* font_face = load_font_face(font);
    configure_font_options(m_font_cairo_state->get_cairo(), font_face, font_size);

    m_font_face = font_face;
    m_loaded_font_face = font;
    m_font_size = font_size;
}

auto wall::CairoFontCache::configure_font_options(cairo_t* cairo, cairo_font_face_t* font_face, double font_size) const -> void {
    auto* font_options = cairo_font_options_create();
    cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
    cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);
    cairo_font_options_set_subpixel_order(font_options, m_subpixel);

    cairo_set_font_options(cairo, font_options);
    cairo_set_font_face(cairo, font_face);
    cairo_set_font_size(cairo, font_size);
    cairo_font_options_destroy(font_options);
}

/*
 * This is taken from i3lock and modified to fit our needs.
 *
 * Returns the cairo_font_face_t for sans-serif
 * if this thing returns NULL, then we're somehow on a system without a
 * sans-serif font, or a system without fontconfig (but with cairo somehow)
 * Reference counts and caches the font-face, so we don't have memory leaks
 * (and so that we don't have to go through all the parsing each time)
 */
auto wall::CairoFontCache::load_font_face(std::string_view font) const -> cairo_font_face_t* {
    FcResult result;

    /*
     * Loads the default config.
     * On successive calls, does no work and just returns true.
     */
    if (FcInit() == FcFalse) {
        LOG_DEBUG("Fontconfig init failed. No text will be shown.");
        return nullptr;
    }

    /*
     * converts a font face name to a pattern for that face name
     */
    auto* pattern = FcNameParse((const unsigned char*)font.data());
    if (pattern == nullptr) {
        LOG_DEBUG("no sans-serif font available");
        return nullptr;
    }

    /*
     * Gets the default font for our pattern. (Gets the default sans-serif font face)
     * Without these two calls, the FcFontMatch call will fail due to FcConfigGetCurrent()
     * not giving it a valid/useful config.
     */
    FcDefaultSubstitute(pattern);
    if (FcConfigSubstitute(FcConfigGetCurrent(), pattern, FcMatchPattern) == FcFalse) {
        LOG_DEBUG("config sub failed?");
        return nullptr;
    }

    /*
     * Looks up the font pattern and does some internal RenderPrepare work,
     * then returns the resulting pattern that's ready for rendering.
     */
    auto* pattern_ready = FcFontMatch(FcConfigGetCurrent(), pattern, &result);

    FcPatternDestroy(pattern);
    pattern = nullptr;
    if (pattern_ready == nullptr) {
        LOG_DEBUG("no sans-serif font available\n");
        return nullptr;
    }

    /*
     * Passes the given pattern into cairo, which loads it into a cairo freetype font face.
     * Increment its reference count and cache it.
     */
    auto* face = cairo_ft_font_face_create_for_pattern(pattern_ready);
    FcPatternDestroy(pattern_ready);
    FcFini();

    return face;
}
