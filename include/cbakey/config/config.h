#pragma once

#include <string>
#include <vector>

#include "cbakey/core/types.h"

namespace cbakey::config {

enum class Fcitx5PreeditMode {
    Auto,
    Client,
    Panel,
};

struct RuntimeConfig {
    cbakey::core::InputMethod method = cbakey::core::InputMethod::Telex;
    bool enableUserDictionary = true;
    bool enableStaticExpansion = true;
    std::string toggleHotkey = "Ctrl+Alt+Z";
    Fcitx5PreeditMode fcitx5PreeditMode = Fcitx5PreeditMode::Auto;
};

RuntimeConfig defaultConfig();
RuntimeConfig loadConfigFile(const std::string& path);
std::vector<std::string> validateConfig(const RuntimeConfig& config);

}  // namespace cbakey::config
