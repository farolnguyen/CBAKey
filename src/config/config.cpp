#include "cbakey/config/config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace cbakey::config {

namespace {

std::string trim(std::string value) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

bool parseBool(const std::string& value, bool defaultValue) {
    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }
    return defaultValue;
}

}  // namespace

RuntimeConfig defaultConfig() {
    return RuntimeConfig{};
}

RuntimeConfig loadConfigFile(const std::string& path) {
    RuntimeConfig config = defaultConfig();
    std::ifstream file(path);
    if (!file.is_open()) {
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        const std::string key = trim(line.substr(0, pos));
        const std::string value = trim(line.substr(pos + 1));

        if (key == "method") {
            if (value == "telex") {
                config.method = cbakey::core::InputMethod::Telex;
            } else if (value == "vni") {
                config.method = cbakey::core::InputMethod::Vni;
            }
        } else if (key == "enable_user_dictionary") {
            config.enableUserDictionary = parseBool(value, config.enableUserDictionary);
        } else if (key == "enable_static_expansion") {
            config.enableStaticExpansion = parseBool(value, config.enableStaticExpansion);
        } else if (key == "toggle_hotkey") {
            config.toggleHotkey = value;
        }
    }

    return config;
}

std::vector<std::string> validateConfig(const RuntimeConfig& config) {
    std::vector<std::string> errors;

    if (config.toggleHotkey.empty()) {
        errors.emplace_back("toggle_hotkey must not be empty");
    }
    if (config.toggleHotkey.find("Ctrl+") == std::string::npos &&
        config.toggleHotkey.find("Alt+") == std::string::npos) {
        errors.emplace_back("toggle_hotkey should include at least one modifier");
    }

    return errors;
}

}  // namespace cbakey::config
