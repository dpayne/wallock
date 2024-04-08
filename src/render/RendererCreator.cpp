#include "render/RendererCreator.hpp"

#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <spdlog/common.h>
#include <wayland-egl-core.h>
#include <array>
#include <exception>
#include <utility>

#include "display/Display.hpp"
#include "mpv/MpvResource.hpp"
#include "render/RendererEGL.hpp"
#include "render/RendererMpv.hpp"
#include "surface/Surface.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
}  // namespace wall
struct wl_surface;

wall::RendererCreator::RendererCreator(const Config& config, Display* display)
    : m_config{config}, m_display{display}, m_egl_display{eglGetDisplay((EGLNativeDisplayType)display->get_wl_display())} {
    // initialize egl
    if (m_egl_display == EGL_NO_DISPLAY) {
        LOG_FATAL("Could not get EGL display");
    }

    EGLint major{};
    EGLint minor{};
    if (eglInitialize(m_egl_display, &major, &minor) != EGL_TRUE) {
        LOG_FATAL("Could not initialize EGL");
    }

    eglBindAPI(EGL_OPENGL_API);

    EGLint num_configs{};
    std::array<EGLint, 13> attribs = {EGL_SURFACE_TYPE,
                                      EGL_WINDOW_BIT,
                                      EGL_RENDERABLE_TYPE,
                                      EGL_OPENGL_BIT,
                                      EGL_RED_SIZE,
                                      8,
                                      EGL_GREEN_SIZE,
                                      8,
                                      EGL_BLUE_SIZE,
                                      8,
                                      EGL_ALPHA_SIZE,
                                      8,
                                      EGL_NONE};
    if (eglChooseConfig(m_egl_display, attribs.data(), &m_egl_config, 1, &num_configs) != EGL_TRUE) {
        LOG_FATAL("Could not choose EGL config");
    }

    m_egl_context = eglCreateContext(m_egl_display, m_egl_config, EGL_NO_CONTEXT, nullptr);
    if (m_egl_context == EGL_NO_CONTEXT) {
        LOG_FATAL("Could not create EGL context");
    }
}

wall::RendererCreator::~RendererCreator() {
    if (m_egl_display == EGL_NO_DISPLAY) {
        return;
    }

    eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_egl_display, m_egl_context);
    eglTerminate(m_egl_display);
}

auto wall::RendererCreator::get_config() const -> const Config& { return m_config; }

auto wall::RendererCreator::get_egl_display() const -> EGLDisplay { return m_egl_display; }

auto wall::RendererCreator::create_egl_surface(wl_surface* surface, uint32_t width, uint32_t height) const -> std::unique_ptr<SurfaceEGL> {
    LOG_DEBUG("Creating EGL surface {} {}", width, height);
    if (surface == nullptr || m_egl_display == EGL_NO_DISPLAY || m_egl_config == nullptr || m_egl_context == EGL_NO_CONTEXT) {
        LOG_FATAL("EGL config is not initialized");
    }

    if (width == 0 || height == 0 || width > static_cast<uint32_t>(INT32_MAX) || height > static_cast<uint32_t>(INT32_MAX)) {
        LOG_FATAL("Invalid width or height");
    }

    LOG_DEBUG("Creating EGL window {} {}", width, height);
    auto* egl_window = wl_egl_window_create(surface, static_cast<int32_t>(width), static_cast<int32_t>(height));
    if (egl_window == nullptr) {
        LOG_FATAL("Couldn't create EGL window");
    }

    LOG_DEBUG("Creating EGL surface");
    auto* egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config, (EGLNativeWindowType)egl_window, nullptr);  // NOLINT
    if (egl_surface == EGL_NO_SURFACE) {
        LOG_FATAL("Couldn't create EGL surface {}", eglGetError());
    }

    LOG_DEBUG("Making context current");
    if (eglMakeCurrent(m_egl_display, egl_surface, egl_surface, m_egl_context) != EGL_TRUE) {
        LOG_FATAL("Couldn't make context current");
    }

    LOG_DEBUG("Setting swap interval");
    eglSwapInterval(m_egl_display, 0);

    return std::make_unique<SurfaceEGL>(m_egl_display, m_egl_context, egl_window, egl_surface);
}

auto wall::RendererCreator::make_current(const SurfaceEGL& surface) const -> void {
    if (eglMakeCurrent(m_egl_display, surface.get_egl_surface(), surface.get_egl_surface(), m_egl_context) != EGL_TRUE) {
        LOG_FATAL("Couldn't make context current");
    }
}

auto wall::RendererCreator::create_egl_renderer(Surface* surface) const -> void {
    auto renderer = std::make_shared<RendererEGL>(get_config(), m_display, m_egl_display, m_egl_context,
                                                  create_egl_surface(surface->get_wl_surface(), surface->get_width(), surface->get_height()));
    surface->set_renderer(std::move(renderer));
}

auto wall::RendererCreator::create_mpv_renderer(Surface* surface) const -> void {
    LOG_DEBUG("Creating mpv renderer for surface {}", surface->get_output_name());

    // make surface current before loading mpv resource
    auto surface_egl = create_egl_surface(surface->get_wl_surface(), surface->get_width(), surface->get_height());
    make_current(*surface_egl);

    if (surface->get_mpv_resource() == nullptr) {
        LOG_DEBUG("Creating mpv resource for surface {}", surface->get_output_name());
        surface->set_mpv_resource(std::make_shared<MpvResource>(get_config(), m_display, surface));

        try {
            surface->get_mpv_resource()->setup();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create mpv resource: {}", e.what());
            surface->set_is_failed(true);
            return;
        }
    }

    auto renderer = std::make_shared<RendererMpv>(get_config(), m_display, m_egl_display, m_egl_context, std::move(surface_egl));
    surface->set_renderer(std::move(renderer));
    surface->get_mpv_resource()->set_surface(surface);
    surface->get_mpv_resource()->play();
}
