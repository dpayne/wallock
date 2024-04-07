#include "overlay/CairoSurface.hpp"

#include <cairo.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <chrono>
#include <cstdint>
#include "display/Display.hpp"
#include "registry/Registry.hpp"
#include "surface/Surface.hpp"
#include "util/Log.hpp"

namespace {
constexpr auto k_pixel_width = 4;
}  // namespace

wall::CairoSurface::CairoSurface(const Config& config, Surface* surface, wl_output_subpixel subpixel)
    : m_config{config}, m_font_cache{config, wl_subpixel_to_cairo_subpixel(subpixel)}, m_surface{surface} {}

wall::CairoSurface::~CairoSurface() {
    if (m_redraw_timer != nullptr) {
        m_redraw_timer->close();
        m_redraw_timer = nullptr;
    }
}

auto wall::CairoSurface::get_font_cache() const -> const CairoFontCache& { return m_font_cache; }

auto wall::CairoSurface::get_font_cache_mut() -> CairoFontCache* { return &m_font_cache; }

auto wall::CairoSurface::get_now() const -> std::chrono::system_clock::time_point { return m_now; }

auto wall::CairoSurface::set_now(std::chrono::system_clock::time_point now) -> void { m_now = now; }

auto wall::CairoSurface::get_last_draw_time() const -> std::chrono::system_clock::time_point { return m_last_draw_time; }

auto wall::CairoSurface::get_last_activity_time() const -> std::chrono::system_clock::time_point { return m_last_activity_time; }

auto wall::CairoSurface::get_cairo_state() const -> CairoState* { return m_cairo_state.get(); }

auto wall::CairoSurface::get_config() const -> const Config& { return m_config; }

auto wall::CairoSurface::get_surface() const -> Surface* { return m_surface; }

auto wall::CairoSurface::get_pixel_width() const -> int32_t { return k_pixel_width; }

auto wall::CairoSurface::wl_subpixel_to_cairo_subpixel(wl_output_subpixel subpixel) -> cairo_subpixel_order_t {
    auto cario_subpixel = CAIRO_SUBPIXEL_ORDER_DEFAULT;
    switch (subpixel) {
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
            cario_subpixel = CAIRO_SUBPIXEL_ORDER_RGB;
            break;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
            cario_subpixel = CAIRO_SUBPIXEL_ORDER_BGR;
            break;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
            cario_subpixel = CAIRO_SUBPIXEL_ORDER_VRGB;
            break;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
            cario_subpixel = CAIRO_SUBPIXEL_ORDER_VBGR;
            break;
        default:
            cario_subpixel = CAIRO_SUBPIXEL_ORDER_DEFAULT;
            break;
    }

    return cario_subpixel;
}

auto wall::CairoSurface::set_last_activity_time(std::chrono::system_clock::time_point now) -> void { m_last_activity_time = now; }

auto wall::CairoSurface::set_last_draw_time(std::chrono::system_clock::time_point now) -> void { m_last_draw_time = now; }

auto wall::CairoSurface::set_idle_timeout(std::chrono::milliseconds idle_timeout) -> void { m_idle_timeout = idle_timeout; }

auto wall::CairoSurface::set_is_visible_on_idle(bool is_visible_on_idle) -> void { m_is_visible_on_idle = is_visible_on_idle; }

auto wall::CairoSurface::set_state(State state) -> void { m_state = state; }

auto wall::CairoSurface::get_state() const -> State { return m_state; }

auto wall::CairoSurface::on_state_change(State state) -> void {
    set_last_draw_time({});
    switch (state) {
        case State::None:
            [[fallthrough]];
        case State::NoOp:
            break;
        case State::Backspace:
        case State::Keypress:
            if (get_state() == State::Idle || get_state() == State::Cleared || get_state() == State::Wrong) {
                set_state(State::Input);
            }
            set_last_activity_time(get_now());
            break;
        case State::Wrong:
            set_last_activity_time(get_now());
            [[fallthrough]];
        case State::Cleared:
            [[fallthrough]];
        case State::Verifying:
            [[fallthrough]];
        case State::Idle:
            [[fallthrough]];
        case State::Valid:
            [[fallthrough]];
        case State::Input:
            [[fallthrough]];
        case State::CapsLock:
            [[fallthrough]];
        default:
            set_state(state);
            break;
    }
}

auto wall::CairoSurface::should_draw_frame_on_idle(wl_surface* surface, wl_subsurface* subsurface) -> bool {
    const auto time_since_last_activity = get_now() - m_last_activity_time;
    if (m_last_activity_time.time_since_epoch().count() != 0 && time_since_last_activity >= m_idle_timeout && m_state != State::Idle &&
        m_state != State::Verifying && m_state != State::Wrong) {
        set_state(State::Idle);
        clear_surface(surface, subsurface);
    }

    return m_state != State::Idle || m_is_visible_on_idle;
}

auto wall::CairoSurface::update_surface(wl_surface* child_surface,
                                        wl_subsurface* subsurface,
                                        int32_t subsurf_xpos,
                                        int32_t subsurf_ypos,
                                        Buffer* buffer) -> void {
    if (child_surface == nullptr || buffer == nullptr) {
        return;
    }

    wl_subsurface_set_position(subsurface, subsurf_xpos, subsurf_ypos);

    wl_surface_set_buffer_scale(child_surface, 1);
    wl_surface_attach(child_surface, buffer->m_buffer, 0, 0);
    wl_surface_damage_buffer(child_surface, 0, 0, INT32_MAX, INT32_MAX);
    wl_surface_commit(child_surface);
    wl_surface_commit(m_surface->get_wl_surface());
}

auto wall::CairoSurface::clear() -> void {
    if (m_cairo_state == nullptr) {
        return;
    }

    auto* cairo = m_cairo_state->get_cairo();
    cairo_set_source_rgba(cairo, 0, 0, 0, 0);
    cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cairo);

    if (m_redraw_timer != nullptr) {
        m_redraw_timer->close();
        m_redraw_timer = nullptr;
    }
}

auto wall::CairoSurface::clear_surface(wl_surface* surface, wl_subsurface* subsurface) -> void {
    // clear buffer
    if (m_cairo_state != nullptr) {
        clear();
        update_surface(surface, subsurface, 0, 0, m_cairo_state->get_buffer());
    }
}

auto wall::CairoSurface::create_cairo_surface(int32_t width, int32_t height, int32_t pixel_size) -> void {
    if (m_cairo_state != nullptr && m_cairo_state->get_width() == width && m_cairo_state->get_height() == height &&
        m_cairo_state->get_pixel_width() == pixel_size) {
        clear();
    } else {
        auto buffer = m_surface->get_registry()->get_buffer_pool_mut()->create_buffer(width, height, pixel_size, WL_SHM_FORMAT_ARGB8888);
        if (buffer == nullptr) {
            LOG_FATAL("Failed to create buffer");
        }

        auto* surface = cairo_image_surface_create_for_data((unsigned char*)buffer->m_data, CAIRO_FORMAT_ARGB32, width, height, pixel_size * width);
        auto* cairo = cairo_create(surface);
        auto cairo_state = std::make_unique<CairoState>(width, height, pixel_size, cairo, surface, std::move(buffer));

        cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

        m_cairo_state = std::move(cairo_state);
    }
}

auto wall::CairoSurface::draw(int32_t width, int32_t height, int32_t scale) -> void {
    m_scale = scale;
    // this happens when we've switched to the main desktop
    if (get_state() == State::Valid) {
        return;
    }

    if (width == 0 || height == 0 || scale == 0) {
        LOG_ERROR("Invalid dimensions for cairo surface {} {} {}", width, height, scale);
        return;
    }

    if (!m_surface->is_ready_to_draw()) {
        LOG_DEBUG("Surface not fully configured yet");
        return;
    }

    m_now = std::chrono::system_clock::now();

    const auto min_redraw_time = draw_frame(width, height, scale);
    setup_redraw_timer(min_redraw_time);
}

auto wall::CairoSurface::setup_redraw_timer(std::chrono::milliseconds min_redraw_time) -> void {
    if (min_redraw_time.count() > 0 && m_surface->get_display() != nullptr && m_surface->get_display()->get_loop() != nullptr) {
        if (m_redraw_timer != nullptr) {
            if (m_redraw_timer->get_expiration() > (m_now + min_redraw_time)) {
                m_redraw_timer->set_expiration(m_now + min_redraw_time);
                m_redraw_timer->set_interval(min_redraw_time);
            }
        } else {
            m_redraw_timer = m_surface->get_display()->get_loop()->add_timer(min_redraw_time, min_redraw_time, [this](loop::Timer* /* timer */) {
                draw(m_surface->get_width(), m_surface->get_height(), m_scale);
            });
        }
    }
}
