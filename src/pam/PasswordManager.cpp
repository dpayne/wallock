#include "pam/PasswordManager.hpp"

#include <pwd.h>
#include <security/pam_appl.h>
#include <spdlog/common.h>
#include <unistd.h>
#include <cstdlib>
#include <utility>

#include "State.hpp"
#include "util/Log.hpp"
#include "util/PasswordBuffer.hpp"

wall::PasswordManager::PasswordManager(Loop* loop) : m_loop{loop}, m_passwd(getpwuid(getuid())), m_username(m_passwd->pw_name) {
    const struct pam_conv conv = {
        .conv = handle_pam_conv_static,
        .appdata_ptr = this,
    };

    m_auth_done_poll = m_loop->add_poll_pipe([this](loop::PollPipe*, const std::vector<uint8_t>&) {
        if (m_password_data != nullptr) {
            m_auth_thread.join();

            m_password_data->m_callback(m_password_data->m_is_authenticated ? State::Valid : State::Wrong);
            delete m_password_data;
            m_password_data = nullptr;
        }
    });

    if (pam_start("wallock", m_username.c_str(), &conv, &m_auth_handle) != PAM_SUCCESS) {
        LOG_FATAL("Failed to start PAM session");
    }
}

wall::PasswordManager::~PasswordManager() { stop(); }

auto wall::PasswordManager::stop() -> void {
    if (m_auth_handle != nullptr) {
        pam_setcred(m_auth_handle, PAM_REFRESH_CRED);
        pam_end(m_auth_handle, PAM_SUCCESS);
        m_auth_handle = nullptr;
    }

    if (m_auth_done_poll != nullptr) {
        m_auth_done_poll->close();
        m_auth_done_poll = nullptr;
    }
}

auto wall::PasswordManager::handle_pam_conv_static(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* data) -> int {
    return static_cast<PasswordManager*>(data)->handle_pam_conv(num_msg, msg, resp);
}

auto wall::PasswordManager::handle_pam_conv(int num_msg, const struct pam_message** msg, struct pam_response** resp) -> int {
    *resp = static_cast<pam_response*>(calloc(num_msg, sizeof(struct pam_response)));
    if (*resp == nullptr) {
        LOG_ERROR("Failed to allocate memory for PAM response");
        return PAM_ABORT;
    }

    for (auto i = 0; i < num_msg; i++) {
        switch (msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_OFF:
                [[fallthrough]];
            case PAM_PROMPT_ECHO_ON:
                resp[i]->resp = m_password->get_password();
                if (resp[i]->resp == nullptr) {
                    LOG_ERROR("Failed to allocate memory for PAM response");
                    return PAM_ABORT;
                }
                break;
            case PAM_ERROR_MSG:
                [[fallthrough]];
            case PAM_TEXT_INFO:
                LOG_INFO("PAM info: {}", msg[i]->msg);
                break;
            default:
                LOG_ERROR("Unknown PAM message type: {}", msg[i]->msg_style);
                break;
        }
    }

    return PAM_SUCCESS;
}

auto wall::PasswordManager::authenticate(std::unique_ptr<PasswordBuffer> password_buffer, std::function<void(State)> callback) -> void {
    if (m_password_data != nullptr) {
        // cleaning up previous authentication
        m_auth_thread.join();
        m_password_data = nullptr;
        return;
    }

    m_password = std::move(password_buffer);
    m_password_data = new PasswordData{std::move(callback), this};

    m_auth_thread = std::thread([password_data = m_password_data] {
        auto* self = password_data->m_self;
        const auto pam_ret = pam_authenticate(self->m_auth_handle, 0);
        password_data->m_is_authenticated = (pam_ret == PAM_SUCCESS);

        if (pam_ret != PAM_SUCCESS) {
            LOG_ERROR("Failed to authenticate user ({}): {}", pam_ret, pam_strerror(self->m_auth_handle, pam_ret));
        }

        LOG_DEBUG("Authentication done");

        self->m_password->release_buffer();  // Pam frees the buffer
        self->m_auth_done_poll->write_one();
    });
}
