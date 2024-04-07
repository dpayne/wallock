#pragma once

#include <mpv/client.h>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "util/Loop.hpp"

namespace wall {

using mpv_event_callback = void (*)(void* data, uint64_t user_event_id);

class MpvEventHandler;

class MpvEventHandlerData {
   public:
    MpvEventHandlerData(MpvEventHandler* mpv_event_handler, uint64_t event_type, mpv_event_callback callback, void* data);
    ~MpvEventHandlerData();

    [[nodiscard]] auto get_id() const -> uint64_t;

    [[nodiscard]] auto get_data() const -> void*;

    auto set_data(void* data) -> void;

    [[nodiscard]] auto get_event_type() const -> uint64_t;

    auto set_event_type(uint64_t event_type) -> void;

    [[nodiscard]] auto get_callback() const -> mpv_event_callback;

    auto set_callback(mpv_event_callback callback) -> void;

   protected:
    [[nodiscard]] auto get_event_handler() const -> MpvEventHandler*;

   private:
    uint64_t m_id{};

    uint64_t m_event_type{};

    mpv_event_callback m_callback{};

    void* m_data{};

    MpvEventHandler* m_event_handler{};
};

class MpvEventHandler {
   public:
    MpvEventHandler(Loop* loop, mpv_handle* mpv);

    ~MpvEventHandler();

    auto add_event_handler(uint64_t event_type, mpv_event_callback callback, void* data) -> std::unique_ptr<MpvEventHandlerData>;

    auto remove_event_handler(uint64_t handler_id) -> void;

    auto stop() -> void;

   protected:
   private:
    static auto wakeup(void* ctx) -> void;

    auto handle_new_events() -> void;

    Loop* m_loop{};

    mpv_handle* m_mpv{};

    loop::PollPipe* m_wakeup_poll{};

    std::unordered_map<uint64_t, MpvEventHandlerData*> m_event_handlers;
};
}  // namespace wall
