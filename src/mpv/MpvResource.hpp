#pragma once

#include <mpv/client.h>
#include <mpv/render.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <filesystem>
#include "mpv/MpvEventHandler.hpp"
#include "mpv/MpvResourceConfig.hpp"
#include "util/Loop.hpp"

namespace wall {

class MpvScreenshot;
class MpvFileLoader;
class Display;
class Surface;

class MpvResource : public std::enable_shared_from_this<MpvResource> {
   public:
    MpvResource(const Config& config, Display* display, Surface* surface);
    ~MpvResource();

    MpvResource(const MpvResource&) = delete;
    auto operator=(const MpvResource&) -> MpvResource& = delete;
    MpvResource(MpvResource&&) = delete;
    auto operator=(MpvResource&&) -> MpvResource& = delete;

    [[nodiscard]] auto get_mpv() const -> mpv_handle*;

    [[nodiscard]] auto get_mpv_context() const -> mpv_render_context*;

    [[nodiscard]] auto get_current_file() const -> const std::filesystem::path&;

    auto setup() -> void;

    auto set_surface(Surface* surface) -> void;

    auto load_new_config(const wall::MpvResourceConfig& new_config) -> void;

    auto terminate() -> void;

    auto play() -> void;

    auto pause() -> void;

    auto stop() -> void;

    auto next() -> void;

    [[nodiscard]] auto get_resource_config() const -> const MpvResourceConfig&;

    auto send_mpv_cmd(const char* arg1) const -> void;

    auto send_mpv_cmd(const char* arg1, const char* arg2) const -> void;

    auto send_mpv_cmd(const char* arg1, const char* arg2, const char* arg3) const -> void;

    [[nodiscard]] auto get_surface() const -> Surface*;

    [[nodiscard]] auto get_display() const -> Display*;

    [[nodiscard]] auto is_single_frame() const -> bool;

   protected:
    auto load_mpv_options() -> void;

    auto setup_event_handlers() -> void;

    auto handle_file_loaded() -> void;

    [[nodiscard]] auto get_config() const -> const Config&;

    virtual auto send_mpv_cmd_base(std::array<const char*, 4> args) const -> void;

    auto setup_update_callback() -> void;

    auto handle_mpv_update() -> void;

   private:
    static auto get_proc_address(void* ctx, const char* name) -> void*;

    const Config& m_config;

    bool m_is_single_frame{false};

    Display* m_display{};

    Surface* m_surface{};

    bool m_is_paused{false};

    loop::PollPipe* m_mpv_update_async{};

    MpvResourceConfig m_resource_config{};

    mpv_handle* m_mpv{};

    mpv_render_context* m_mpv_context{};

    std::unique_ptr<MpvEventHandler> m_event_handler;

    std::vector<std::unique_ptr<MpvEventHandlerData>> m_event_handlers{};

    std::shared_ptr<MpvScreenshot> m_screenshot;

    std::unique_ptr<MpvFileLoader> m_file_loader;
};

}  // namespace wall
