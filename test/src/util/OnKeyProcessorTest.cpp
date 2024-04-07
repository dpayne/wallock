#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "conf/ConfigDefaultSettings.hpp"
#include "pam/PasswordManager.hpp"
#include "util/OnKeyProcessor.hpp"
#include "util/PasswordBuffer.hpp"

class PasswordManagerMock : public wall::PasswordManager {
   public:
    PasswordManagerMock(wall::Loop* loop) : wall::PasswordManager(loop) {}

    MOCK_METHOD(void, authenticate, (std::unique_ptr<wall::PasswordBuffer>, std::function<void(wall::State)>), (override));
};

class OnKeyProcessorMock : public wall::OnKeyProcessor {
   public:
    OnKeyProcessorMock(const wall::Config& config,
                       std::unique_ptr<wall::PasswordManager> password_manager,
                       std::function<void(wall::State)> on_state_change)
        : wall::OnKeyProcessor(config, std::move(password_manager), std::move(on_state_change)) {}

    [[nodiscard]] auto get_password_buffer() const -> wall::PasswordBuffer* { return wall::OnKeyProcessor::get_password_buffer(); }
};

class OnKeyProcessorTest : public ::testing::Test {
   public:
    OnKeyProcessorTest() : m_config{wall::Config::get_default_config()} {}

   protected:
    void SetUp() override {
        auto password_manager = std::make_unique<PasswordManagerMock>(&m_loop);
        m_password_manager = password_manager.get();
        m_config = wall::Config::get_default_config();
        m_state = wall::State::None;
        m_on_key_processor =
            std::make_unique<OnKeyProcessorMock>(m_config, std::move(password_manager), [this](wall::State state) { on_state_change(state); });
    }

    void on_state_change(wall::State state) { m_state = state; }

    wall::Config m_config;
    wall::Loop m_loop{};
    PasswordManagerMock* m_password_manager{};
    wall::State m_state{wall::State::None};
    std::unique_ptr<OnKeyProcessorMock> m_on_key_processor;
};

TEST_F(OnKeyProcessorTest, submit_empty_password) {
    m_config.set(wall::conf::k_password_allow_empty, false);
    EXPECT_CALL(*m_password_manager, authenticate(testing::_, testing::_)).Times(0);
    m_on_key_processor->on_key(XKB_KEY_Return, "");
    EXPECT_EQ(m_state, wall::State::None);

    m_on_key_processor->on_key(XKB_KEY_KP_Enter, "");
    EXPECT_EQ(m_state, wall::State::None);
}

TEST_F(OnKeyProcessorTest, allow_empty_password) {
    m_config.set(wall::conf::k_password_allow_empty, true);
    EXPECT_CALL(*m_password_manager, authenticate(testing::_, testing::_)).Times(1);
    EXPECT_STREQ("", m_on_key_processor->get_password_buffer()->get_password());
    m_on_key_processor->on_key(XKB_KEY_Return, "");
    EXPECT_EQ(m_state, wall::State::Verifying);
}

TEST_F(OnKeyProcessorTest, allow_empty_password_enter) {
    m_config.set(wall::conf::k_password_allow_empty, true);
    EXPECT_CALL(*m_password_manager, authenticate(testing::_, testing::_)).Times(1);
    m_on_key_processor->on_key(XKB_KEY_KP_Enter, "");
    EXPECT_EQ(m_state, wall::State::Verifying);
}

TEST_F(OnKeyProcessorTest, handle_no_key) {
    m_on_key_processor->on_key(XKB_KEY_NoSymbol, "");
    EXPECT_EQ(m_state, wall::State::NoOp);
}

TEST_F(OnKeyProcessorTest, key_press) {
    m_config.set(wall::conf::k_password_allow_empty, false);
    EXPECT_CALL(*m_password_manager, authenticate(testing::_, testing::_)).Times(1);

    m_on_key_processor->on_key(XKB_KEY_p, "p");

    EXPECT_EQ(m_state, wall::State::Keypress);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "p");

    m_on_key_processor->on_key(XKB_KEY_Return, "");
    EXPECT_EQ(m_state, wall::State::Verifying);
}

TEST_F(OnKeyProcessorTest, key_backspace) {
    m_config.set(wall::conf::k_password_allow_empty, false);
    EXPECT_CALL(*m_password_manager, authenticate(testing::_, testing::_)).Times(1);

    m_on_key_processor->on_key(XKB_KEY_BackSpace, "");
    EXPECT_EQ(m_state, wall::State::Cleared);

    m_on_key_processor->on_key(XKB_KEY_p, "p");

    EXPECT_EQ(m_state, wall::State::Keypress);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "p");

    m_on_key_processor->on_key(XKB_KEY_a, "a");

    EXPECT_EQ(m_state, wall::State::Keypress);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "pa");

    m_on_key_processor->on_key(XKB_KEY_Delete, "");
    EXPECT_EQ(m_state, wall::State::Backspace);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "p");

    m_on_key_processor->on_key(XKB_KEY_Return, "");
    EXPECT_EQ(m_state, wall::State::Verifying);
}

TEST_F(OnKeyProcessorTest, key_backspace_non_ascii) {
    m_config.set(wall::conf::k_password_allow_empty, false);
    EXPECT_CALL(*m_password_manager, authenticate(testing::_, testing::_)).Times(1);

    m_on_key_processor->on_key(XKB_KEY_BackSpace, "");
    EXPECT_EQ(m_state, wall::State::Cleared);

    m_on_key_processor->on_key(XKB_KEY_p, "p");

    EXPECT_EQ(m_state, wall::State::Keypress);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "p");

    m_on_key_processor->on_key(XKB_KEY_a, " ");

    EXPECT_EQ(m_state, wall::State::Keypress);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "p ");

    m_on_key_processor->on_key(XKB_KEY_Delete, "");
    EXPECT_EQ(m_state, wall::State::Backspace);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "p");

    m_on_key_processor->on_key(XKB_KEY_Return, "");
    EXPECT_EQ(m_state, wall::State::Verifying);
}

TEST_F(OnKeyProcessorTest, key_clear) {
    m_on_key_processor->on_key(XKB_KEY_Escape, "");
    EXPECT_EQ(m_state, wall::State::Cleared);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "");

    m_on_key_processor->on_key(XKB_KEY_p, "p");
    m_on_key_processor->on_key(XKB_KEY_a, "a");
    EXPECT_EQ(m_state, wall::State::Keypress);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "pa");

    m_on_key_processor->on_key(XKB_KEY_Escape, "");
    EXPECT_EQ(m_state, wall::State::Cleared);
    EXPECT_STREQ(m_on_key_processor->get_password_buffer()->get_password(), "");
}
