#include "registry/Registry.hpp"
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include "display/Display.hpp"
#include "display/Screen.hpp"
#include "registry/BufferPool.hpp"
#include "util/Log.hpp"
#include "wlr-layer-shell-unstable-v1-protocol.h"

const wl_registry_listener wall::Registry::k_listener = {
    .global =
        [](void* data, wl_registry* /* registry */, uint32_t name, const char* interface, uint32_t version) {
            auto& self = *static_cast<wall::Registry*>(data);
            self.on_global(name, interface, version);
        },
    .global_remove =
        [](void* data, wl_registry* /* registry */, uint32_t name) {
            auto& self = *static_cast<wall::Registry*>(data);
            self.on_global_remove(name);
        },
};

wall::Registry::Registry(const Config& config, Display* display, Loop* loop, wl_registry* registry)
    : m_config{config},
      m_display{display},
      m_loop{loop},
      m_registry{registry},
      m_compositor{std::make_unique<Compositor>(nullptr)},
      m_subcompositor{std::make_unique<Subcompositor>(nullptr)},
      m_layer_shell{std::make_unique<LayerShell>(nullptr)},
      m_lock_manager{std::make_unique<LockManager>(nullptr)},
      m_xdg_wm_base{std::make_unique<XdgBase>(nullptr)},
      m_buffer_pool{std::make_unique<BufferPool>()},
      m_seat{std::make_unique<Seat>(loop, nullptr)} {
    if (m_registry == nullptr) {
        LOG_ERROR("Failed to get registry");
        return;
    }

    wl_registry_add_listener(m_registry, &k_listener, this);
}

wall::Registry::~Registry() {
    // Order matters here, registry must come last
    m_buffer_pool = nullptr;
    m_xdg_wm_base = nullptr;
    m_lock_manager = nullptr;
    m_layer_shell = nullptr;
    m_subcompositor = nullptr;
    m_compositor = nullptr;
    m_seat = nullptr;

    if (m_registry != nullptr) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
}

auto wall::Registry::get_loop() const -> Loop* { return m_loop; }

auto wall::Registry::get_config() const -> const Config& { return m_config; }

auto wall::Registry::on_global(uint32_t name, const char* interface, uint32_t version) -> void {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        m_compositor->set_wl_compositor(static_cast<wl_compositor*>(wl_registry_bind(m_registry, name, &wl_compositor_interface, version)));
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        m_buffer_pool->set_shm(static_cast<wl_shm*>(wl_registry_bind(m_registry, name, &wl_shm_interface, version)));
    } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
        m_subcompositor->set_wl_subcompositor(
            static_cast<wl_subcompositor*>(wl_registry_bind(m_registry, name, &wl_subcompositor_interface, version)));
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        auto* wl_seat = static_cast<struct wl_seat*>(wl_registry_bind(m_registry, name, &wl_seat_interface, version));
        m_seat->set_wl_seat(wl_seat);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        // Create a screen for every output
        auto* output = static_cast<wl_output*>(wl_registry_bind(m_registry, name, &wl_output_interface, version));
        m_screens.emplace_back(std::make_unique<Screen>(get_config(), m_display, this, name, output));
    } else if (strcmp(interface, ext_session_lock_manager_v1_interface.name) == 0) {
        m_lock_manager->set_lock_manager(
            static_cast<ext_session_lock_manager_v1*>(wl_registry_bind(m_registry, name, &ext_session_lock_manager_v1_interface, version)));
    } else if (strcmp(interface, zwlr_input_inhibit_manager_v1_interface.name) == 0) {
        m_input_inhibit_manager =
            static_cast<zwlr_input_inhibit_manager_v1*>(wl_registry_bind(m_registry, name, &zwlr_input_inhibit_manager_v1_interface, version));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        m_xdg_wm_base->set_wm_base(static_cast<xdg_wm_base*>(wl_registry_bind(m_registry, name, &xdg_wm_base_interface, version)));
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        m_layer_shell->set_wlr_layer_shell(
            static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(m_registry, name, &zwlr_layer_shell_v1_interface, version)));
    } else {
        LOG_DEBUG("Unknown global: {} {} {}", name, interface, version);
    }
}

auto wall::Registry::move_screens() -> std::vector<std::unique_ptr<wall::Screen>> { return std::move(m_screens); }

auto wall::Registry::on_global_remove(uint32_t name) -> void {
    LOG_DEBUG("Global removed: {}", name);
    m_display->remove_screen(name);
}

auto wall::Registry::get_lock() -> std::unique_ptr<Lock> {
    if (m_lock_manager == nullptr || m_lock_manager->get_lock_manager() == nullptr) {
        LOG_ERROR("Lock manager is not available");
        return nullptr;
    }

    return std::make_unique<Lock>(ext_session_lock_manager_v1_lock(m_lock_manager->get_lock_manager()));
}

auto wall::Registry::get_screens_mut() -> std::vector<std::unique_ptr<wall::Screen>>* { return &m_screens; }

auto wall::Registry::get_screens() const -> const std::vector<std::unique_ptr<wall::Screen>>& { return m_screens; }

auto wall::Registry::destory_screens() -> void { m_screens.clear(); }
