#include "conf/ConfigGenerator.hpp"

#include <iostream>

auto wall::ConfigGenerator::generate_example(std::ostream& output,
                                             const std::vector<std::pair<std::string, conf::SettingsVariantType>>& options,
                                             const std::unordered_map<std::string, std::string>& descriptions) -> void {
    for (const auto& [key, value] : options) {
        const auto type = get_type_name(key);
        const auto description_iter = descriptions.find(key);
        const auto description = description_iter != descriptions.end() ? description_iter->second : "";
        if (!description.empty()) {
            output << "\n# " << description << '\n';
        }
        output << key << "=" << get_value_string(value) << '\n';
    }
}

auto wall::ConfigGenerator::generate_example_markdown(std::ostream& output,
                                                      const std::vector<std::pair<std::string, conf::SettingsVariantType>>& options,
                                                      const std::unordered_map<std::string, std::string>& descriptions) -> void {
    output << "| Key | Default | Description |\n";
    for (const auto& [key, value] : options) {
        const auto description_iter = descriptions.find(key);
        const auto description = description_iter != descriptions.end() ? description_iter->second : "";
        output << "| " << key << " | " << get_value_string(value) << " | " << description << " |\n";
    }
}

auto wall::ConfigGenerator::get_type_name(const conf::SettingsVariantType& value) -> std::string {
    if (std::holds_alternative<bool>(value)) {
        return "boolean";
    }

    if (std::holds_alternative<double>(value)) {
        return "float";
    }

    if (std::holds_alternative<unsigned long>(value)) {
        return "unsigned integer";
    }

    if (std::holds_alternative<long>(value)) {
        return "integer";
    }

    if (std::holds_alternative<std::string>(value)) {
        return "string";
    }

    return "";
}

auto wall::ConfigGenerator::get_value_string(const conf::SettingsVariantType& value) -> std::string {
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }

    if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    }

    if (std::holds_alternative<unsigned long>(value)) {
        return std::to_string(std::get<unsigned long>(value));
    }

    if (std::holds_alternative<long>(value)) {
        return std::to_string(std::get<long>(value));
    }

    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }

    return "";
}
