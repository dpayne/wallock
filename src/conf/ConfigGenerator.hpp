#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "conf/ConfigDefaultSettings.hpp"

namespace wall {
class ConfigGenerator {
   public:
    static auto generate_example(std::ostream& output,
                                 const std::vector<std::pair<std::string, conf::SettingsVariantType>>& options,
                                 const std::unordered_map<std::string, std::string>& descriptions) -> void;

    static auto generate_example_markdown(std::ostream& output,
                                          const std::vector<std::pair<std::string, conf::SettingsVariantType>>& options,
                                          const std::unordered_map<std::string, std::string>& descriptions) -> void;

   protected:
   private:
    static auto get_type_name(const conf::SettingsVariantType& value) -> std::string;

    static auto get_value_string(const conf::SettingsVariantType& value) -> std::string;
};
}  // namespace wall
