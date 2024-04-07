#pragma once

#include <EGL/egl.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>

namespace wall {
class SurfaceEGL {
   public:
    SurfaceEGL(EGLDisplay egl_display, EGLContext egl_context, struct wl_egl_window* egl_window, EGLSurface egl_surface);

    ~SurfaceEGL();

    SurfaceEGL(const SurfaceEGL&) = delete;
    auto operator=(const SurfaceEGL&) -> SurfaceEGL& = delete;
    SurfaceEGL(SurfaceEGL&&) = delete;
    auto operator=(SurfaceEGL&&) -> SurfaceEGL& = delete;

    [[nodiscard]] auto get_egl_surface() const -> EGLSurface;

    [[nodiscard]] auto get_egl_window() const -> struct wl_egl_window*;

   private:
    EGLDisplay m_egl_display{};

    EGLContext m_egl_context{};

    struct wl_egl_window* m_egl_window{};

    EGLSurface m_egl_surface{};
};
}  // namespace wall
