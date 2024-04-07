#include "render/RendererMpv.hpp"

#include <mpv/render_gl.h>
#include <wayland-client-protocol.h>
#include <array>
#include "display/Display.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "surface/Surface.hpp"
#include "surface/SurfaceEGL.hpp"

wall::RendererMpv::RendererMpv(const Config& config,
                               Display* display,
                               EGLDisplay egl_display,
                               EGLContext egl_context,
                               std::unique_ptr<SurfaceEGL> surface_egl,
                               MpvResource* resource)
    : Renderer(config, display, std::move(surface_egl)), m_egl_display{egl_display}, m_egl_context{egl_context}, m_resource{resource} {}

wall::RendererMpv::~RendererMpv() { RendererMpv::stop(); };

auto wall::RendererMpv::should_render(Surface* surface) -> bool {
    if (m_resource == nullptr) {
        return false;
    }

    if (!is_dirty()) {
        return false;
    }

    if (!surface->is_configured()) {
        return false;
    }

    if (is_callback_scheduled()) {
        return false;
    }

    if (get_surface_egl_mut() == nullptr) {
        return false;
    }

    return true;
}

auto wall::RendererMpv::render(Surface* surface) -> void {
    if (!should_render(surface)) {
        return;
    }

    mpv_opengl_fbo fbo_params = {
        .fbo = 0, .w = static_cast<int32_t>(surface->get_width()), .h = static_cast<int32_t>(surface->get_height()), .internal_format = 0};

    auto one = 1;
    auto zero = 1;
    std::array<mpv_render_param, 4> render_params = {mpv_render_param{MPV_RENDER_PARAM_OPENGL_FBO, &fbo_params},
                                                     // Flip rendering (needed due to flipped GL coordinate system).
                                                     mpv_render_param{MPV_RENDER_PARAM_FLIP_Y, &one},
                                                     // Do not wait for a fresh frame to render
                                                     {MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &zero},
                                                     mpv_render_param{MPV_RENDER_PARAM_INVALID, nullptr}};

    if (m_resource->get_mpv_context() != nullptr) {
        if (eglMakeCurrent(m_egl_display, get_surface_egl().get_egl_surface(), get_surface_egl().get_egl_surface(), m_egl_context) != EGL_TRUE) {
            LOG_ERROR("Couldn't make context current {}", eglGetError());
            return;
        }

        // Render frame
        auto err_code = mpv_render_context_render(m_resource->get_mpv_context(), render_params.data());
        if (err_code < 0) {
            LOG_ERROR("Couldn't render frame: {} for {}", err_code, surface->get_output_name());
            return;
        }

        setup_next_frame_callback(surface);
        set_is_dirty(false);

        if (eglSwapBuffers(m_egl_display, get_surface_egl().get_egl_surface()) == EGL_FALSE) {
            LOG_ERROR("Couldn't swap buffers {} for {}", eglGetError(), surface->get_output_name());

            if (surface->get_display()->is_nvidia()) {
                // nvidia has issues with race conditions when setting up egl surfaces.
                set_is_recreate_egl_surface(true);
                set_is_dirty(true);
                Renderer::stop();  // kill the next callback so that we can force a render
            }
            return;
        }
        surface->draw_overlay();
    }
}

auto wall::RendererMpv::reload_resource(wall::ResourceMode mode) -> void {
    if (m_resource != nullptr) {
        const auto new_config = MpvResourceConfig::build_config(get_config(), mode);
        m_resource->load_new_config(new_config);
    }
}

auto wall::RendererMpv::stop() -> void {
    Renderer::stop();
    if (m_resource != nullptr) {
        m_resource = nullptr;
    }
}

auto wall::RendererMpv::pause() -> void {
    if (m_resource != nullptr) {
        m_resource->pause();
    }
}

auto wall::RendererMpv::play() -> void {
    if (m_resource != nullptr) {
        m_resource->pause();
    }
}

auto wall::RendererMpv::next() -> void {
    if (m_resource != nullptr) {
        m_resource->next();
    }
}
