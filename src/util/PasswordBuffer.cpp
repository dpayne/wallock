#include "util/PasswordBuffer.hpp"

#include <spdlog/common.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "util/Log.hpp"

wall::PasswordBuffer::PasswordBuffer(uint64_t buffer_size) {
    if (buffer_size == 0UL) {
        LOG_FATAL("Password buffer size cannot be 0");
    }
    m_page_size = get_page_size();
    m_password_buffer_size = buffer_size;
    m_password_buffer = create(m_password_buffer_size);
    mem_lock(m_password_buffer, m_password_buffer_size);
    m_password_buffer[0] = '\0';
}

wall::PasswordBuffer::~PasswordBuffer() {
    if (m_password_buffer != nullptr) {
        mem_unlock(m_password_buffer, m_password_buffer_size);
        free(m_password_buffer);
    }
}

auto wall::PasswordBuffer::release_buffer() -> void { m_password_buffer = nullptr; }

auto wall::PasswordBuffer::empty() const -> bool { return m_password_size == 0UL; }

auto wall::PasswordBuffer::get_password() const -> char* { return m_password_buffer; }

auto wall::PasswordBuffer::clear() -> void {
    m_password_size = 0UL;
    m_password_parts_sizes.clear();
    m_password_buffer[0] = '\0';
}

auto wall::PasswordBuffer::push(const std::string& str_part) -> void {
    if ((str_part.size() + m_password_size + 1) >= m_password_buffer_size) {
        LOG_FATAL("Password longer than allowed {} {} {}", str_part.size(), m_password_size, m_password_buffer_size);
        return;
    }

    memcpy(m_password_buffer + m_password_size, str_part.c_str(), str_part.size());
    m_password_parts_sizes.push_back(str_part.size());
    m_password_size += str_part.size();
    m_password_buffer[m_password_size] = '\0';
}
auto wall::PasswordBuffer::pop() -> void {
    if (m_password_parts_sizes.empty()) {
        return;
    }

    auto last_size = m_password_parts_sizes.back();
    if (last_size > m_password_size) {
        m_password_size = 0;
    } else {
        m_password_size -= last_size;
    }
    m_password_parts_sizes.pop_back();
    m_password_buffer[m_password_size] = '\0';
}

auto wall::PasswordBuffer::get_page_size() const -> uint64_t { return sysconf(_SC_PAGESIZE); }

auto wall::PasswordBuffer::create(uint64_t size) const -> char* {
    char* password_buffer = nullptr;
    const auto resp = posix_memalign((void**)&password_buffer, m_page_size, size);
    if (resp != 0) {
        LOG_FATAL("Could not mem lock paassword buffer")
    }
    return password_buffer;
}

auto wall::PasswordBuffer::mem_lock(char* buffer, uint64_t buffer_size) const -> bool {
    auto retries = 5;
    while (mlock((void*)buffer, buffer_size) != 0 && retries > 0) {
        switch (errno) {
            case EAGAIN:
                retries--;
                if (retries == 0) {
                    LOG_FATAL("mlock() supported but failed too often.");
                }
                break;
            case EPERM:
                LOG_ERROR("Unable to mlock() password memory: Unsupported!");
                return true;
            default:
                LOG_ERROR("Unable to mlock() password memory.");
                return false;
        }
    }

    return true;
}

auto wall::PasswordBuffer::mem_unlock(char* buffer, uint64_t buffer_size) const -> void {
    if (m_is_mem_lock_supported) {
        if (munlock((void*)buffer, buffer_size) != 0) {
            LOG_ERROR("Unable to munlock() password memory.");
        }
    }
}
