#pragma once

#include <string>
#include <vector>

#include "cbakey/core/types.h"

namespace cbakey::config {

struct RuntimeConfig {
    cbakey::core::InputMethod method = cbakey::core::InputMethod::Telex;
    bool enableUserDictionary = true;
    bool enableStaticExpansion = true;
    std::string toggleHotkey = "Ctrl+Alt+Z";
};

RuntimeConfig defaultConfig();
RuntimeConfig loadConfigFile(const std::string& path);
std::vector<std::string> validateConfig(const RuntimeConfig& config);

}  // namespace cbakey::config
