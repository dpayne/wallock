#include "render/RendererMpv.hpp"

#include <EGL/egl.h>
#include <mpv/render_gl.h>
#include <wayland-client-protocol.h>
#include <array>
#include "display/Display.hpp"
#include "surface/Surface.hpp"
#include "surface/SurfaceEGL.hpp"

wall::RendererMpv::RendererMpv(const Config& config,
                               Display* display,
                               EGLDisplay egl_display,
                               EGLContext egl_context,
                               std::unique_ptr<SurfaceEGL> surface_egl)
    : Renderer(config, display, std::move(surface_egl)), m_egl_display{egl_display}, m_egl_context{egl_context} {}

wall::RendererMpv::~RendererMpv() { RendererMpv::stop(); };

auto wall::RendererMpv::should_render(Surface* surface) -> bool {
    if (surface->get_mpv_resource() == nullptr) {
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

    auto* resource = surface->get_mpv_resource();
    if (resource->get_mpv_context() != nullptr) {
        if (eglMakeCurrent(m_egl_display, get_surface_egl().get_egl_surface(), get_surface_egl().get_egl_surface(), m_egl_context) != EGL_TRUE) {
            LOG_ERROR("Couldn't make context current {}", eglGetError());
            return;
        }

        // Render frame
        auto err_code = mpv_render_context_render(resource->get_mpv_context(), render_params.data());
        if (err_code < 0) {
            LOG_ERROR("Couldn't render frame: {} for {}", err_code, surface->get_output_name());
            return;
        }

        setup_next_frame_callback(surface);
        set_is_dirty(false);
        err_code = eglGetError();
        if (err_code != EGL_SUCCESS) {
            LOG_ERROR("Error after rendering: {}", err_code);
        }

        if (eglSwapBuffers(m_egl_display, get_surface_egl().get_egl_surface()) == EGL_FALSE) {
            LOG_ERROR("Couldn't swap buffers {} for {}", eglGetError(), surface->get_output_name());
            return;
        }

        set_has_buffer(true);
        surface->draw_overlay();
    }
}
