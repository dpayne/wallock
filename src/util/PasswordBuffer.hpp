#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wall {
class PasswordBuffer {
   public:
    explicit PasswordBuffer(uint64_t buffer_size);
    ~PasswordBuffer();

    PasswordBuffer(PasswordBuffer&&) = delete;
    PasswordBuffer(const PasswordBuffer&) = delete;
    auto operator=(const PasswordBuffer&) -> PasswordBuffer = delete;
    auto operator=(PasswordBuffer&&) -> PasswordBuffer = delete;

    auto pop() -> void;

    auto push(const std::string& str_part) -> void;

    auto clear() -> void;

    [[nodiscard]] auto empty() const -> bool;

    [[nodiscard]] auto get_password() const -> char*;

    auto release_buffer() -> void;

   protected:
    [[nodiscard]] auto create(uint64_t size) const -> char*;

    [[nodiscard]] auto get_page_size() const -> uint64_t;

    auto mem_lock(char* buffer, uint64_t buffer_size) const -> bool;

    auto mem_unlock(char* buffer, uint64_t buffer_size) const -> void;

   private:
    uint64_t m_page_size{};

    char* m_password_buffer{};

    uint64_t m_password_buffer_size{};

    bool m_is_mem_lock_supported{true};

    std::vector<uint64_t> m_password_parts_sizes;

    uint64_t m_password_size{};
};

}  // namespace wall
