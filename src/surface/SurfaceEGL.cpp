#include "surface/SurfaceEGL.hpp"

#include <wayland-egl-core.h>
#include "util/Log.hpp"

wall::SurfaceEGL::SurfaceEGL(EGLDisplay egl_display, EGLContext egl_context, struct wl_egl_window* egl_window, EGLSurface egl_surface)
    : m_egl_display{egl_display}, m_egl_context{egl_context}, m_egl_window{egl_window}, m_egl_surface{egl_surface} {}

wall::SurfaceEGL::~SurfaceEGL() {
    if (m_egl_surface != EGL_NO_SURFACE) {
        if (eglDestroySurface(eglGetCurrentDisplay(), m_egl_surface) != EGL_TRUE) {
            LOG_ERROR("Failed to destroy EGL surface");
        }
        m_egl_surface = EGL_NO_SURFACE;
    }

    if (m_egl_window != nullptr) {
        wl_egl_window_destroy(m_egl_window);
        m_egl_window = nullptr;
    }
}

auto wall::SurfaceEGL::get_egl_surface() const -> EGLSurface { return m_egl_surface; }

auto wall::SurfaceEGL::get_egl_window() const -> struct wl_egl_window* { return m_egl_window; }
