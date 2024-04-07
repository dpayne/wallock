#pragma once

#include <chrono>
#include <string>

namespace wall {
class TestUtils {
   public:
    [[maybe_unused]] static auto convert_date_string_to_time_point(const std::string& dateStr) -> std::chrono::time_point<std::chrono::system_clock>;
};
}  // namespace wall
