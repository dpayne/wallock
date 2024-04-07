#pragma once

#include <EGL/egl.h>
#include <wayland-client.h>
#include <memory>
#include "render/Renderer.hpp"
#include "surface/Surface.hpp"
#include "surface/SurfaceEGL.hpp"

namespace wall {
class MpvResource;
class Display;
class RendererCreator {
   public:
    RendererCreator(const Config& config, Display* display);

    ~RendererCreator();

    RendererCreator(const RendererCreator&) = delete;
    auto operator=(const RendererCreator&) -> RendererCreator& = delete;
    RendererCreator(RendererCreator&&) = delete;
    auto operator=(RendererCreator&&) -> RendererCreator& = delete;

    auto create_egl_renderer(Surface* surface) const -> void;

    auto create_mpv_renderer(Surface* surface) const -> void;

    [[nodiscard]] auto get_egl_display() const -> EGLDisplay;

    [[nodiscard]] auto create_egl_surface(wl_surface* surface, uint32_t width, uint32_t height) const -> std::unique_ptr<SurfaceEGL>;

   protected:
    auto make_current(const SurfaceEGL& surface) const -> void;

    [[nodiscard]] auto get_config() const -> const Config&;

   private:
    const Config& m_config;

    Display* m_display{};

    EGLDisplay m_egl_display{};

    EGLConfig m_egl_config{};

    EGLContext m_egl_context{};
};
}  // namespace wall
