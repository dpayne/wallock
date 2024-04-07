#pragma once

#include <pwd.h>
#include <security/pam_appl.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "State.hpp"
#include "util/Loop.hpp"
#include "util/PasswordBuffer.hpp"

namespace wall {
class PasswordManager {
   public:
    PasswordManager(Loop* loop);
    virtual ~PasswordManager();

    virtual auto authenticate(std::unique_ptr<PasswordBuffer> password_buffer, std::function<void(State)> callback) -> void;

    auto stop() -> void;

   protected:
   private:
    struct PasswordData {
        std::function<void(State)> m_callback;
        PasswordManager* m_self;
        bool m_is_authenticated{false};
    };
    static auto handle_pam_conv_static(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* data) -> int;

    auto handle_pam_conv(int num_msg, const struct pam_message** msg, struct pam_response** resp) -> int;

    Loop* m_loop;

    loop::PollPipe* m_auth_done_poll{};

    std::unique_ptr<PasswordBuffer> m_password;

    PasswordData* m_password_data{};

    std::thread m_auth_thread;

    struct passwd* m_passwd{};

    std::string m_username;

    pam_handle_t* m_auth_handle{};
};
}  // namespace wall
