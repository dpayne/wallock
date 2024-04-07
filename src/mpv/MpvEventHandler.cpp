#include "mpv/MpvEventHandler.hpp"

#include <mpv/client.h>
#include <spdlog/common.h>
#include <atomic>
#include <cstdint>

namespace {
std::atomic_uint64_t g_id = 0UL;
}

wall::MpvEventHandlerData::MpvEventHandlerData(MpvEventHandler* mpv_event_handler, uint64_t event_type, mpv_event_callback callback, void* data)
    : m_id{g_id++}, m_event_type{event_type}, m_callback{callback}, m_data{data}, m_event_handler{mpv_event_handler} {}

wall::MpvEventHandlerData::~MpvEventHandlerData() { m_event_handler->remove_event_handler(m_id); }

auto wall::MpvEventHandlerData::get_id() const -> uint64_t { return m_id; }

auto wall::MpvEventHandlerData::get_data() const -> void* { return m_data; }

auto wall::MpvEventHandlerData::set_data(void* data) -> void { m_data = data; }

auto wall::MpvEventHandlerData::get_event_type() const -> uint64_t { return m_event_type; }

auto wall::MpvEventHandlerData::set_event_type(uint64_t event_type) -> void { m_event_type = event_type; }

auto wall::MpvEventHandlerData::get_event_handler() const -> MpvEventHandler* { return m_event_handler; }

auto wall::MpvEventHandlerData::get_callback() const -> mpv_event_callback { return m_callback; }

auto wall::MpvEventHandlerData::set_callback(mpv_event_callback callback) -> void { m_callback = callback; }

wall::MpvEventHandler::MpvEventHandler(Loop* loop, mpv_handle* mpv) : m_loop{loop}, m_mpv{mpv} {
    m_wakeup_poll = m_loop->add_poll_pipe([this](loop::Poll*, const std::vector<uint8_t>& /* buffer */) { handle_new_events(); });

    mpv_set_wakeup_callback(m_mpv, wakeup, this);
}

wall::MpvEventHandler::~MpvEventHandler() {
    mpv_set_wakeup_callback(m_mpv, nullptr, nullptr);
    if (m_wakeup_poll != nullptr) {
        m_wakeup_poll->close();
        m_wakeup_poll = nullptr;
    }
}

auto wall::MpvEventHandler::stop() -> void {
    if (m_wakeup_poll != nullptr) {
        m_wakeup_poll->close();
        m_wakeup_poll = nullptr;
    }
}

auto wall::MpvEventHandler::wakeup([[maybe_unused]] void* ctx) -> void {
    auto* event_handler = static_cast<MpvEventHandler*>(ctx);

    if (event_handler->m_wakeup_poll != nullptr) {
        // wake up might happen off the main thread, so we need to send an event back to the main event loop to process the events
        event_handler->m_wakeup_poll->write_one();
    }
}

auto wall::MpvEventHandler::handle_new_events() -> void {
    mpv_event* event = mpv_wait_event(m_mpv, 0);
    while (event != nullptr && event->event_id != MPV_EVENT_NONE) {
        for (auto& [id, handler] : m_event_handlers) {
            if (handler->get_event_type() == event->event_id) {
                handler->get_callback()(handler->get_data(), event->reply_userdata);
            }
        }
        event = mpv_wait_event(m_mpv, 0);
    }
}

auto wall::MpvEventHandler::add_event_handler(uint64_t event_type, mpv_event_callback callback, void* data) -> std::unique_ptr<MpvEventHandlerData> {
    auto event_handler_data = std::make_unique<MpvEventHandlerData>(this, event_type, callback, data);
    m_event_handlers[event_handler_data->get_id()] = event_handler_data.get();
    return event_handler_data;
}

auto wall::MpvEventHandler::remove_event_handler(uint64_t handler_id) -> void { m_event_handlers.erase(handler_id); }
