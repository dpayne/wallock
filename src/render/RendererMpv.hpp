#pragma once

#include <EGL/egl.h>
#include <memory>
#include "mpv/MpvResource.hpp"
#include "render/Renderer.hpp"

namespace wall {
class SurfaceEGL;
class RendererMpv : public Renderer {
   public:
    RendererMpv(const Config& config, Display* display, EGLDisplay egl_display, EGLContext egl_context, std::unique_ptr<SurfaceEGL> surface_egl);

    ~RendererMpv() override;

    RendererMpv(const RendererMpv&) = delete;
    auto operator=(const RendererMpv&) -> RendererMpv& = delete;
    RendererMpv(RendererMpv&&) = delete;
    auto operator=(RendererMpv&&) -> RendererMpv& = delete;

    auto render(Surface* surface) -> void override;

   protected:
    auto should_render(Surface* surface) -> bool;

   private:
    EGLDisplay m_egl_display{};

    EGLContext m_egl_context{};
};
}  // namespace wall
