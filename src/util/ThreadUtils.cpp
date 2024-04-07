#include "util/ThreadUtils.hpp"

#include <sstream>
#include <thread>

auto wall::ThreadUtils::get_thread_id() -> std::string {
    const auto thread_id = std::this_thread::get_id();
    std::stringstream str_stream;
    str_stream << thread_id;

    return str_stream.str();
}
