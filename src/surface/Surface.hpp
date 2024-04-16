#pragma once

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <cstdint>
#include <filesystem>
#include <memory>
#include "State.hpp"
#include "mpv/MpvResource.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "render/Renderer.hpp"

namespace wall {
class CairoBarSurface;
class CairoIndicatorSurface;
class RendererCreator;
class MpvResource;
class Display;
class Registry;

class Surface {
   public:
    Surface(const Config& config, std::string output_name, Display* display, Registry* registry, wl_output* output);
    virtual ~Surface();

    Surface(const Surface&) = delete;
    auto operator=(const Surface&) -> Surface& = delete;
    Surface(Surface&&) = delete;
    auto operator=(Surface&&) -> Surface& = delete;

    auto update_settings() -> void;
    virtual auto on_configure([[maybe_unused]] uint32_t serial, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height) -> void {}

    auto next() -> void;

    [[nodiscard]] auto get_wl_output() const -> wl_output*;

    [[nodiscard]] auto get_wl_surface() const -> wl_surface*;

    [[nodiscard]] auto get_wl_indicator_subsurface() const -> wl_subsurface*;

    [[nodiscard]] auto get_wl_indicator_surface() const -> wl_surface*;

    [[nodiscard]] auto get_wl_bar_subsurface() const -> wl_subsurface*;

    [[nodiscard]] auto get_wl_bar_surface() const -> wl_surface*;

    [[nodiscard]] auto get_width() const -> uint32_t;

    [[nodiscard]] auto get_height() const -> uint32_t;

    [[nodiscard]] auto get_scale_factor() const -> int32_t;

    [[nodiscard]] auto get_renderer() const -> const Renderer&;

    [[nodiscard]] auto get_renderer_mut() -> Renderer*;

    [[nodiscard]] auto get_subpixel() const -> wl_output_subpixel;

    [[nodiscard]] virtual auto get_resource_mode() const -> ResourceMode = 0;

    auto set_renderer(std::shared_ptr<Renderer> renderer) -> void;

    [[nodiscard]] auto is_valid() const -> bool;

    auto set_scale_factor(int32_t scale_factor) -> void;

    auto set_width(uint32_t width) -> void;

    auto set_height(uint32_t height) -> void;

    virtual auto draw_overlay() -> void;

    auto set_subpixel(wl_output_subpixel subpixel) -> void;

    auto on_state_change(State state) -> void;

    [[nodiscard]] auto get_mpv_resource() const -> MpvResource*;

    [[nodiscard]] auto share_mpv_resource() -> std::shared_ptr<MpvResource>;

    auto set_mpv_resource(std::shared_ptr<MpvResource> resource) -> void;

    [[nodiscard]] auto copy_mpv_resource() -> std::shared_ptr<MpvResource>;

    [[nodiscard]] auto get_output_name() const -> const std::string&;

    [[nodiscard]] auto is_primary() const -> bool;

    auto set_is_primary(bool is_primary) -> void;

    [[nodiscard]] auto get_registry() const -> Registry*;

    [[nodiscard]] auto get_display() const -> Display*;

    [[nodiscard]] auto is_configured() const -> bool;

    [[nodiscard]] virtual auto is_ready_to_draw() -> bool;

    [[nodiscard]] auto is_failed() const -> bool;

    auto set_is_failed(bool is_failed) -> void;

    virtual auto destroy_resources() -> void;

    [[nodiscard]] auto get_next_resource_override() const -> const std::filesystem::path&;

    auto set_next_resource_override(std::filesystem::path next_resource_override) -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

    auto create_surface() -> void;

    [[nodiscard]] auto get_indicator() const -> CairoIndicatorSurface*;

    [[nodiscard]] auto get_bar() const -> CairoBarSurface*;

    auto set_is_configured(bool is_configured) -> void;

   private:
    const Config& m_config;

    bool m_is_configured{false};

    std::string m_output_name;

    bool m_is_primary{false};

    bool m_is_failed{false};

    Display* m_display{};

    Registry* m_registry{};

    std::shared_ptr<MpvResource> m_mpv_resource{};

    uint32_t m_width{};

    uint32_t m_height{};

    int32_t m_scale_factor{};

    wl_output* m_output{};

    wl_surface* m_surface{};

    wl_subsurface* m_indicator_subsurface{};

    wl_surface* m_indicator_surface{};

    wl_subsurface* m_bar_subsurface{};

    wl_surface* m_bar_surface{};

    wl_output_subpixel m_subpixel{};

    std::shared_ptr<Renderer> m_renderer;

    std::unique_ptr<CairoIndicatorSurface> m_indicator{nullptr};

    std::unique_ptr<CairoBarSurface> m_bar{nullptr};

    std::filesystem::path m_next_resource_override{};
};
}  // namespace wall
