#pragma once

#include <string>

namespace wall {

class ThreadUtils {
   public:
    [[maybe_unused]] static auto get_thread_id() -> std::string;
};
}  // namespace wall
