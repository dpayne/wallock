#include "render/RendererEGL.hpp"

#include <GL/gl.h>
#include <spdlog/common.h>
#include <wayland-client-core.h>
#include <utility>
#include "surface/Surface.hpp"
#include "surface/SurfaceEGL.hpp"
#include "util/Log.hpp"

namespace wall {
class Config;
}  // namespace wall

wall::RendererEGL::RendererEGL(const Config& config,
                               Display* display,
                               EGLDisplay egl_display,
                               EGLContext egl_context,
                               std::unique_ptr<SurfaceEGL> surface_egl)
    : Renderer(config, display, std::move(surface_egl)), m_egl_display{egl_display}, m_egl_context{egl_context} {}

auto wall::RendererEGL::render([[maybe_unused]] Surface* surface) -> void {
    if (eglMakeCurrent(m_egl_display, get_surface_egl().get_egl_surface(), get_surface_egl().get_egl_surface(), m_egl_context) == EGL_FALSE) {
        LOG_FATAL("Couldn't make EGL context current");
        return;
    }

    static double red = 0.0;
    static double green = 79.0;
    static double blue = 158.0;

    glClearColor(red / 255.0, green / 255.0, blue / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable blending and set blending function if you're rendering semi-transparent content
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);

    red += 1.0;
    green += 1.0;
    blue += 1.0;

    red = fmod(red, 255.0);
    green = fmod(green, 255.0);
    blue = fmod(blue, 255.0);

    setup_next_frame_callback(surface);
    eglSwapBuffers(m_egl_display, get_surface_egl().get_egl_surface());
    set_has_buffer(true);
}
