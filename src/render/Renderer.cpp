#include "render/Renderer.hpp"

#include <spdlog/common.h>
#include <wayland-client-protocol.h>
#include <utility>

#include "display/Display.hpp"
#include "surface/Surface.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
enum class ResourceMode;
}  // namespace wall
struct wl_callback;

const wl_callback_listener wall::Renderer::k_frame_listener = {
    .done =
        [](void* data, wl_callback* callback, uint32_t /* time */) {
            wl_callback_destroy(callback);
            auto* callback_data = static_cast<FrameCallbackData*>(data);
            if (callback_data->m_is_valid && callback_data->m_renderer != nullptr && callback_data->m_surface != nullptr) {
                callback_data->m_renderer->m_last_callback_data = nullptr;
                callback_data->m_renderer->m_last_callback = nullptr;
                callback_data->m_renderer->render(callback_data->m_surface);
            } else {
                // happens when the callback is destroyed before it is called
                LOG_DEBUG("Callback data is invalid");
            }
            delete callback_data;
        },
};

wall::Renderer::Renderer(const Config& config, Display* display, std::unique_ptr<SurfaceEGL> surface_egl)
    : m_config{config}, m_display{display}, m_egl_surface{std::move(surface_egl)} {}

wall::Renderer::~Renderer() { Renderer::stop(); }

auto wall::Renderer::stop() -> void {
    // cancel last callback
    if (m_last_callback_data != nullptr) {
        m_last_callback_data->m_is_valid = false;
        m_last_callback_data->m_renderer = nullptr;
        wl_callback_destroy(m_last_callback);
        delete m_last_callback_data;
        m_last_callback_data = nullptr;
        m_last_callback = nullptr;
    }
}

auto wall::Renderer::reload_resource(wall::ResourceMode /* mode */) -> void {}

auto wall::Renderer::pause() -> void {}

auto wall::Renderer::play() -> void {}

auto wall::Renderer::next() -> void {}

auto wall::Renderer::get_config() const -> const Config& { return m_config; }

auto wall::Renderer::set_surface_egl(std::unique_ptr<SurfaceEGL> egl_surface) -> void { m_egl_surface = std::move(egl_surface); }

auto wall::Renderer::move_egl_surface() -> std::unique_ptr<SurfaceEGL> { return std::move(m_egl_surface); }

auto wall::Renderer::get_surface_egl() const -> const SurfaceEGL& { return *m_egl_surface; }

auto wall::Renderer::get_surface_egl_mut() -> SurfaceEGL* { return m_egl_surface.get(); }

auto wall::Renderer::set_is_dirty(bool is_dirty) -> void { m_is_dirty = is_dirty; }

auto wall::Renderer::is_dirty() const -> bool { return m_is_dirty; }

auto wall::Renderer::has_buffer() const -> bool { return m_has_buffer; }

auto wall::Renderer::set_has_buffer(bool has_buffer) -> void { m_has_buffer = has_buffer; }

auto wall::Renderer::is_callback_scheduled() const -> bool { return m_last_callback != nullptr; }

auto wall::Renderer::is_recreate_egl_suface() const -> bool { return m_is_recreate_egl_surface; }

auto wall::Renderer::set_is_recreate_egl_surface(bool is_recreate_egl_surface) -> void { m_is_recreate_egl_surface = is_recreate_egl_surface; }

auto wall::Renderer::setup_next_frame_callback(Surface* surface) -> void {
    if (m_last_callback != nullptr) {
        return;
    }
    // Callback new frame
    m_last_callback = wl_surface_frame(surface->get_wl_surface());

    static auto frame_number = 0UL;
    auto* callback_data = new FrameCallbackData{surface, this, true, this, frame_number++};
    m_last_callback_data = callback_data;
    wl_callback_add_listener(m_last_callback, &k_frame_listener, callback_data);
}
