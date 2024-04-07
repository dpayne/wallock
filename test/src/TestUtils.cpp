#include "TestUtils.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>

auto wall::TestUtils::convert_date_string_to_time_point(const std::string& dateStr) -> std::chrono::time_point<std::chrono::system_clock> {
    std::tm time_st = {};
    std::istringstream string_stream(dateStr);

    // Assuming dateStr is in the format "2001-01-01 12:00:00"
    string_stream >> std::get_time(&time_st, "%Y-%m-%d %H:%M:%S");
    if (string_stream.fail()) {
        throw std::runtime_error("Failed to parse date string");
    }

    // Convert std::tm to time_t, then to time_point
    return std::chrono::system_clock::from_time_t(std::mktime(&time_st));
}
