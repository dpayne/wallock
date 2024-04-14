#pragma once

#include <wayland-client.h>
#include <memory>
#include "conf/Config.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "surface/SurfaceEGL.hpp"

namespace wall {
class Surface;
class Display;

class Renderer : public std::enable_shared_from_this<Renderer> {
   public:
    Renderer(const Config& config, Display* display, std::unique_ptr<SurfaceEGL> surface_egl);
    virtual ~Renderer();

    Renderer(const Renderer&) = delete;
    auto operator=(const Renderer&) -> Renderer& = delete;
    Renderer(Renderer&&) = delete;
    auto operator=(Renderer&&) -> Renderer& = delete;

    virtual auto render(Surface* surface) -> void = 0;

    auto set_is_dirty(bool is_dirty) -> void;

    [[nodiscard]] auto is_dirty() const -> bool;

    virtual auto stop() -> void;

    virtual auto reload_resource(wall::ResourceMode mode) -> void;

    [[nodiscard]] auto get_surface_egl() const -> const SurfaceEGL&;

    [[nodiscard]] auto get_surface_egl_mut() -> SurfaceEGL*;

    auto set_surface_egl(std::unique_ptr<SurfaceEGL> egl_surface) -> void;

    auto move_egl_surface() -> std::unique_ptr<SurfaceEGL>;

    auto setup_next_frame_callback(Surface* surface) -> void;

    auto set_has_buffer(bool has_buffer) -> void;

    auto has_buffer() const -> bool;

    auto is_recreate_egl_surface() const -> bool;

    auto set_is_recreate_egl_surface(bool is_recreate_egl_surface) -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    [[nodiscard]] auto is_callback_scheduled() const -> bool;

   private:
    struct FrameCallbackData {
        Surface* m_surface{};
        Renderer* m_renderer{};
        bool m_is_valid{false};
        Renderer* m_shared_renderer;
        uint64_t m_frame_number{0};
    };

    static const wl_callback_listener k_frame_listener;

    const Config& m_config;

    bool m_is_dirty{true};

    bool m_has_buffer{false};

    bool m_is_recreate_egl_surface{false};

    Display* m_display{};

    std::unique_ptr<SurfaceEGL> m_egl_surface{};

    struct wl_callback* m_last_callback{};
    struct FrameCallbackData* m_last_callback_data{};
};
}  // namespace wall
