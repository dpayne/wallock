#include "registry/BufferPool.hpp"

#include <fcntl.h>
#include <spdlog/common.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>

#include "util/Log.hpp"

struct wl_buffer;
struct wl_shm;

namespace {
constexpr auto k_stride = 4;
constexpr auto k_file_prefix = "/wall-shared-";

const wl_buffer_listener k_buffer_listener = {
    .release =
        [](void* data, wl_buffer* /* buffer */) {
            auto* buffer = static_cast<wall::Buffer*>(data);
            buffer->m_is_free = true;
        },
};

}  // namespace

wall::BufferSt::~BufferSt() {
    if (m_buffer != nullptr) {
        wl_buffer_destroy(m_buffer);
    }

    if (m_data != nullptr) {
        munmap(m_data, m_size);
    }
}

wall::BufferPool::BufferPool() = default;

auto wall::BufferPool::set_shm(wl_shm* shm) -> void { m_shm = shm; }

auto wall::BufferPool::get_shm() const -> wl_shm* { return m_shm; }

wall::BufferPool::~BufferPool() {
    if (m_shm != nullptr) {
        wl_shm_destroy(m_shm);
    }
}

auto wall::BufferPool::create_anonymous_file() -> int {
    std::string temp;
    int32_t file{};
    auto attempts = 0U;
    while (attempts < 10) {
        temp = k_file_prefix + std::to_string(rand());
        auto* tmp_file = const_cast<char*>(temp.c_str());

        file = shm_open(tmp_file, O_RDWR | O_CREAT | O_EXCL, 600);
        if (file >= 0) {
            LOG_DEBUG("Created anonymous file: {}", tmp_file);
            shm_unlink(tmp_file);
            return file;
        }
        ++attempts;
    }

    LOG_ERROR("Failed to create anonymous file {}, error ({}): {}", file, errno, strerror(errno));

    return file;
}

auto wall::BufferPool::create_buffer(int32_t width, int32_t height, int32_t stride, uint32_t fmt) -> std::unique_ptr<Buffer> {
    if (m_shm == nullptr) {
        return nullptr;
    }

    const auto size = width * height * k_stride;
    auto buffer = std::make_unique<Buffer>();

    if (size > 0) {
        const auto shm_file = create_anonymous_file();
        if (shm_file == -1) {
            return nullptr;
        }

        if (ftruncate(shm_file, size) < 0) {
            close(shm_file);
            return nullptr;
        }

        buffer->m_size = size;
        buffer->m_data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_file, 0);
        auto* pool = wl_shm_create_pool(m_shm, shm_file, size);
        buffer->m_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * stride, fmt);
        wl_buffer_add_listener(buffer->m_buffer, &k_buffer_listener, buffer.get());
        wl_shm_pool_destroy(pool);
        close(shm_file);
    }

    return buffer;
}
