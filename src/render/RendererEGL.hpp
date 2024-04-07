#pragma once

#include <EGL/egl.h>
#include <wayland-client.h>
#include <memory>
#include "render/Renderer.hpp"

namespace wall {
class Surface;
class SurfaceEGL;
class RendererEGL : public Renderer {
   public:
    RendererEGL(const Config& config, Display* display, EGLDisplay egl_display, EGLContext egl_context, std::unique_ptr<SurfaceEGL> surface_egl);
    ~RendererEGL() override = default;

    RendererEGL(const RendererEGL&) = delete;
    auto operator=(const RendererEGL&) -> RendererEGL& = delete;
    RendererEGL(RendererEGL&&) = delete;
    auto operator=(RendererEGL&&) -> RendererEGL& = delete;

    auto render(Surface* surface) -> void override;

   protected:
   private:
    EGLDisplay m_egl_display{};
    EGLContext m_egl_context{};
};
}  // namespace wall
