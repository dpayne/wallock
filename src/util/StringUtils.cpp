#include "util/StringUtils.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

auto wall::StringUtils::trim(std::string_view str) -> std::string { return wall::StringUtils::trim(std::string{str}); }

auto wall::StringUtils::trim(const std::string& str) -> std::string {
    const auto start = std::find_if_not(str.begin(), str.end(), isspace);
    const auto end = std::find_if_not(str.rbegin(), str.rend(), isspace).base();

    const auto dist = std::distance(start, end);
    return dist <= 0 ? std::string{} : std::string(start, end);
}

auto wall::StringUtils::split(const std::string& str, char delimiter) -> std::vector<std::string> {
    if (str.empty()) {
        return {""};
    }

    std::vector<std::string> result;
    std::stringstream string_stream(str);
    std::string segment;

    while (std::getline(string_stream, segment, delimiter)) {
        result.push_back(segment);
    }

    if (str.back() == delimiter) {
        result.emplace_back("");
    }

    return result;
}

auto wall::StringUtils::split_and_trim(const std::string& str, char delimiter) -> std::vector<std::string> {
    auto result = split(str, delimiter);
    std::transform(result.begin(), result.end(), result.begin(), [](const std::string& str1) { return trim(str1); });

    return result;
}

auto wall::StringUtils::to_upper(const std::string& str) -> std::string {
    std::string result;
    result.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(result), [](unsigned char some_char) { return std::toupper(some_char); });

    return result;
}
