#include "conf/Config.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include "conf/ConfigDefaultSettings.hpp"
#include "conf/ConfigMacros.hpp"
#include "util/FileUtils.hpp"
#include "util/StringUtils.hpp"
#include "wallock/version.h"

namespace {
const std::regex k_color_scheme_regex{R"(^\{color(\d+)\})"};

const std::set<std::string_view> k_valid_colors = {
    wall::conf::k_color_0,  wall::conf::k_color_1,          wall::conf::k_color_2,         wall::conf::k_color_3,  wall::conf::k_color_4,
    wall::conf::k_color_5,  wall::conf::k_color_6,          wall::conf::k_color_7,         wall::conf::k_color_8,  wall::conf::k_color_9,
    wall::conf::k_color_10, wall::conf::k_color_11,         wall::conf::k_color_12,        wall::conf::k_color_13, wall::conf::k_color_14,
    wall::conf::k_color_15, wall::conf::k_color_background, wall::conf::k_color_foreground};
}  // namespace

wall::Config::Config(int argc, const char** argv) {
    conf::setup_settings();
    auto options = get_command_line_options();
    if (argc > 0 && argv != nullptr) {
        auto cmd_line_result = options.parse(argc, argv);
        process_cmd_line_args(options, cmd_line_result, m_options);
        m_cmd_line_options = m_options;  // used for reloading
    }

    load_options();
}

auto wall::Config::load_options() -> void {
    for (const auto& [key, value] : conf::g_default_settings) {
        if (!m_options.contains(key)) {
            m_options.insert({key, value});
        }
    }

    read_config_file(m_options, m_options_set_from_config);
    load_color_scheme_file();
    replace_color_scheme_colors();
}

auto wall::Config::reload_options() -> void {
    m_options = m_cmd_line_options;
    m_options_set_from_config.clear();
    load_options();
}

// This constructor is only used in unit tests
wall::Config::Config() {
    conf::setup_settings();
    auto options = get_command_line_options();

    for (const auto& [key, value] : conf::g_default_settings) {
        m_options.insert({key, value});
    }
}

auto wall::Config::get_default_config() -> Config {
    Config config;

    std::istringstream config_stream{""};
    read_config(config_stream, config.m_options, config.m_options_set_from_config);
    config.replace_color_scheme_colors();
    return config;
}

auto wall::Config::read_config_file(OptionsMap& config_options, std::set<std::string>& options_set_from_config) -> void {
    auto find_result = config_options.find(conf::k_config_file);
    const std::string config_file_name_orig =
        find_result != config_options.end() ? std::get<std::string>(find_result->second) : conf::k_default_config_file;

    auto config_file = open_file_for_reading(config_file_name_orig);
    if (!config_file.has_value() || !config_file.value().is_open()) {
        return;
    }

    read_config(config_file.value(), config_options, options_set_from_config);
}

auto wall::Config::open_file_for_reading(std::string_view filename) -> std::optional<std::ifstream> {
    const auto config_file_path_opt = FileUtils::get_expansion_config(std::filesystem::path(filename));
    if (!config_file_path_opt.has_value() || !std::filesystem::exists(config_file_path_opt.value())) {
#ifdef DEBUG
        std::cout << "Config file {} does not exist " << filename << "\n";
#endif
        // no config file exists, use defaults for everything
        return std::nullopt;
    }

    const auto& config_file_path = config_file_path_opt.value();
    if (!std::filesystem::is_regular_file(config_file_path)) {
        std::cerr << "Config file " << config_file_path << " invalid"
                  << "\n";
        return std::nullopt;
    }

#ifdef DEBUG
    std::cout << "Reading config file " << config_file_path.string() << "\n";
#endif

    return std::ifstream{config_file_path};
}

auto wall::Config::load_color_scheme_file() -> void {
    const auto color_scheme_file = StringUtils::trim(wall_conf_get(*this, color_scheme, file));

    auto config_file = open_file_for_reading(color_scheme_file);
    if (!config_file.has_value() || !config_file.value().is_open()) {
        return;
    }

    read_config(config_file.value(), m_options, m_options_set_from_config);
}

auto wall::Config::replace_color_scheme_colors() -> void {
    for (auto& [key, value] : m_options) {
        // colors have to be strings, so skip everything else
        if (!std::holds_alternative<std::string>(value)) {
            continue;
        }

        auto color_name = get_color_name(std::get<std::string>(value));
        if (!color_name.empty()) {
            value = color_name;
        }
    }
}

auto wall::Config::get_color_name(const std::string& value) const -> std::string {
    const auto value_str = StringUtils::trim(value);

    std::string color_key;
    std::string replace_me;
    if (value_str.starts_with("{background}")) {
        color_key = "color_background";
        replace_me = "{background}";
    }

    if (value_str.starts_with("{foreground}")) {
        color_key = "color_foreground";
        replace_me = "{foreground}";
    }

    std::smatch match;
    if (std::regex_search(value_str, match, k_color_scheme_regex)) {
        const auto color_digits = match[1];
        color_key = "color_";
        color_key.append(color_digits);
        replace_me = match[0];
    }

    const auto find_result = m_options.find(color_key);
    if (k_valid_colors.contains(color_key) && find_result != m_options.end()) {
        // get string
        const auto color_value = std::get<std::string>(find_result->second);

        auto result = color_value;
        result.append(value_str.substr(replace_me.size()));

        return result;
    }

    return "";
}

auto wall::Config::read_config(std::istream& config_file, OptionsMap& config_options, std::set<std::string>& options_set_from_config) -> void {
    std::string line;
    while (std::getline(config_file, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        auto key = line.substr(0, pos);
        key = StringUtils::trim(key);
        auto value = line.substr(pos + 1);

        process_config_pair(key, value, config_options, options_set_from_config);
    }
}

auto wall::Config::is_exit_immediately_option(const char* arg) -> bool {
    return std::string{arg} == "--no-daemonize" || std::string{arg} == "-d" || std::string{arg} == "-v" || std::string{arg} == "--version" ||
           std::string{arg} == "-o" || std::string{arg} == "--command" || std::string{arg} == "-h" || std::string{arg} == "--help" ||
           std::string{arg} == "--example-config" || std::string{arg} == "--example-markdown-config";
}

auto wall::Config::process_config_pair(const std::string& key,
                                       const std::string& value,
                                       OptionsMap& config_options,
                                       std::set<std::string>& options_set_from_config) -> void {
    auto result = config_options.find(key);
    if (result == config_options.end()) {
        return;
    }

    if (std::holds_alternative<bool>(result->second)) {
        if (value == "true") {
            result->second = true;
        } else if (value == "false") {
            result->second = false;
        } else {
            LOG_FATAL("Invalid value for key {} in config file", key);
        }
    } else if (std::holds_alternative<long>(result->second)) {
        result->second = std::stol(value);
    } else if (std::holds_alternative<unsigned long>(result->second)) {
        result->second = std::stoul(value);
    } else if (std::holds_alternative<double>(result->second)) {
        result->second = std::stod(value);
    } else if (std::holds_alternative<std::string>(result->second)) {
        result->second = value;
    } else {
        LOG_FATAL("Unsupported type in Config lookup for {}", key);
    }
    options_set_from_config.insert(std::string{key});
}

auto wall::Config::process_cmd_line_args(const cxxopts::Options& options, const cxxopts::ParseResult& result, OptionsMap& config_options) -> void {
    if (result.count("help") > 0) {
        std::cout << options.help() << "\n";
        exit(0);
    }

    if (result.count("version") > 0) {
        std::cout << "Wallock, version " << WALLOCK_LIB_VERSION << "\n";
        exit(0);
    }

    if (result.count("example-config") > 0) {
        conf::print_default_config(std::cout);
        exit(0);
    }

    if (result.count("example-markdown-config") > 0) {
        conf::print_default_config_markdown(std::cout);
        exit(0);
    }

    if (result.count("log") > 0) {
        config_options[conf::k_log_level] = result["log"].as<std::string>();
    }

    if (result.count("config") > 0) {
        config_options[conf::k_config_file] = result["config"].as<std::string>();
    }

    if (result.count("debug") > 0) {
        config_options[conf::k_debug_mode] = result["debug"].as<bool>();
    }

    if (result.count("command") > 0) {
        config_options[conf::k_command_name] = result["command"].as<std::string>();
    }

    if (result.count("start_lock") > 0) {
        config_options[conf::k_start_lock] = true;
    }

    if (result.count("no-daemonize") > 0) {
        config_options[conf::k_general_daemonize] = false;
    }
}

auto wall::Config::get_command_line_options() -> cxxopts::Options {
    cxxopts::Options options("wallock", "Wallock");

    // clang-format off
    options.add_options()
        ("h,help", "Print help")
        ("v,version", "Print version")
        ("no-daemonize", "Do not daemonize, run in foreground")
#ifdef DEBUG
        ("d,debug", "Debug mode",cxxopts::value<bool>())
        ("example-config", "Print example config")
        ("example-markdown-config", "Print config defaults in markdown")
#endif
        ("c,config", "Config file", cxxopts::value<std::string>()->default_value(conf::k_default_config_file))
        ("l,log", "Log level", cxxopts::value<std::string>()->default_value(conf::k_default_log_level))
        ("s,start_lock", "Start lock immediately")
        ("o,command", "Send a command (e.g. -o lock), possible commands are: lock, stop, next, reload, refresh", cxxopts::value<std::string>()->default_value(conf::k_default_log_level));
    // clang-format on

    return options;
}
