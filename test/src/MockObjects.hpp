#pragma once

#include <sys/mman.h>

#include "conf/Config.hpp"
#include "input/Keyboard.hpp"
#include "registry/BufferPool.hpp"
#include "registry/Registry.hpp"
#include "surface/Surface.hpp"

class BufferPoolMock : public wall::BufferPool {
   public:
    BufferPoolMock() = default;
    auto create_buffer(int32_t width, int32_t height, int32_t stride, uint32_t /* fmt */) -> std::unique_ptr<wall::Buffer> override {
        auto buffer = std::make_unique<wall::BufferSt>();
        buffer->m_size = width * height * stride;

        // create a buffer with mmap
        buffer->m_data = mmap(nullptr, buffer->m_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        return buffer;
    }
};

class KeyboardMock : public wall::Keyboard {
   public:
    KeyboardMock(wall::Loop* loop) : wall::Keyboard(loop, nullptr) {}

    [[nodiscard]] auto get_layout() const -> const std::string& override { return m_layout; }

    [[nodiscard]] auto is_caps_lock() const -> bool override { return m_is_caps_lock; }

    bool m_is_caps_lock{};
    std::string m_layout{};
};

class SeatMock : public wall::Seat {
   public:
    SeatMock(wall::Loop* loop) : wall::Seat(loop, nullptr) {}

    [[nodiscard]] auto get_keyboard() const -> const wall::Keyboard& override { return *m_keyboard; }

    [[nodiscard]] auto get_keyboard_mut() -> wall::Keyboard* override { return m_keyboard; }

    auto set_keyboard(wall::Keyboard* keyboard) -> void { m_keyboard = keyboard; }

   private:
    wall::Keyboard* m_keyboard{};
};

class RegistryMock : public wall::Registry {
   public:
    RegistryMock(const wall::Config& config, wall::Loop* loop) : wall::Registry(config, nullptr, loop, nullptr) {}

    [[nodiscard]] auto get_seat() const -> const wall::Seat& override { return *m_seat; }

    [[nodiscard]] auto get_seat_mut() -> wall::Seat* override { return m_seat; }

    auto set_seat(wall::Seat* seat) -> void { m_seat = seat; }

    [[nodiscard]] auto get_buffer_pool() const -> const wall::BufferPool& override { return *m_buffer_pool; }

    [[nodiscard]] auto get_buffer_pool_mut() -> wall::BufferPool* override { return m_buffer_pool; }

    auto set_buffer_pool(wall::BufferPool* buffer_pool) -> void { m_buffer_pool = buffer_pool; }

   private:
    wall::Seat* m_seat{};
    wall::BufferPool* m_buffer_pool{};
};

class SurfaceMock : public wall::Surface {
   public:
    SurfaceMock(const wall::Config& config, wall::Display* display, wall::Registry* registry)
        : wall::Surface(config, "Name", display, registry, nullptr) {
        set_is_configured(true);
    }

    [[nodiscard]] auto get_resource_mode() const -> wall::ResourceMode override { return m_resource_mode; }

    [[nodiscard]] auto is_ready_to_draw() -> bool override { return true; }

    wall::ResourceMode m_resource_mode{wall::ResourceMode::Lock};
};
