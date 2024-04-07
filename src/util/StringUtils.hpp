#ifndef WALL_STRINGUTILS_HPP
#define WALL_STRINGUTILS_HPP

#include <string>
#include <string_view>
#include <vector>

namespace wall {
class StringUtils {
   public:
    static auto trim(const std::string& str) -> std::string;

    static auto trim(std::string_view str) -> std::string;

    static auto to_upper(const std::string& str) -> std::string;

    static auto split(const std::string& str, char delimiter) -> std::vector<std::string>;

    static auto split_and_trim(const std::string& str, char delimiter) -> std::vector<std::string>;

   protected:
   private:
};
}  // namespace wall
#endif  // WALL_STRINGUTILS_HPP
