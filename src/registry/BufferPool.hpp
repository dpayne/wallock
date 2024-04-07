#pragma once

#include <wayland-client-protocol.h>
#include <memory>

namespace wall {

struct BufferSt {
    size_t m_size{};        // NOLINT
    wl_buffer* m_buffer{};  // NOLINT
    bool m_is_free{false};  // NOLINT
    void* m_data{};         // NOLINT

    BufferSt() = default;

    BufferSt(BufferSt&& other) = default;
    auto operator=(BufferSt&& other) -> BufferSt& = default;

    BufferSt(const BufferSt& other) = delete;
    auto operator=(const BufferSt& other) -> BufferSt& = delete;

    ~BufferSt();
};

using Buffer = struct BufferSt;

class BufferPool {
   public:
    explicit BufferPool();
    virtual ~BufferPool();

    BufferPool(BufferPool&& other) = default;
    auto operator=(BufferPool&& other) -> BufferPool& = default;

    BufferPool(const BufferPool& other) = delete;
    auto operator=(const BufferPool& other) -> BufferPool& = delete;

    virtual auto create_buffer(int32_t width, int32_t height, int32_t stride, uint32_t fmt) -> std::unique_ptr<Buffer>;

    auto set_shm(wl_shm* shm) -> void;

    [[nodiscard]] auto get_shm() const -> wl_shm*;

   protected:
    static auto create_anonymous_file() -> int;

   private:
    wl_shm* m_shm{};
};
}  // namespace wall
