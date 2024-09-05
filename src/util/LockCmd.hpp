#pragma once

#include "conf/Config.hpp"
namespace wall {
class LockCmd {
   public:
    explicit LockCmd(const Config& config);
    virtual ~LockCmd() = default;

    LockCmd(LockCmd&&) = delete;
    LockCmd(const LockCmd&) = delete;
    auto operator=(const LockCmd&) -> LockCmd& = delete;
    auto operator=(LockCmd&&) -> LockCmd& = delete;

    auto run() -> void;

    auto stop() -> void;

   protected:
    [[nodiscard]] auto get_config() const -> const Config&;

   private:
    const Config& config;

    pid_t m_lock_process = -1;
};
}  // namespace wall
