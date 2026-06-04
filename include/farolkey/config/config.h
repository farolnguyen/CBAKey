#pragma once

#include <string>
#include <vector>

#include "farolkey/core/free_layout_config.h"
#include "farolkey/core/types.h"

namespace farolkey::config {

enum class Fcitx5PreeditMode {
    Auto,
    Client,
    Panel,
};

struct RuntimeConfig {
    farolkey::core::InputMethod method = farolkey::core::InputMethod::Telex;
    bool enableUserDictionary = true;
    bool enableStaticExpansion = true;
    /// M8.1: path to user dictionary JSON file. Empty = use default XDG path at runtime.
    std::string userDictPath;
    std::string toggleHotkey = "Ctrl+Alt+Z";
    Fcitx5PreeditMode fcitx5PreeditMode = Fcitx5PreeditMode::Auto;
    /// M6.3a: rewrite syllable left of caret via SurroundingText + deleteSurroundingText. Off by
    /// default because many clients report unreliable surrounding/cursor (Chromium, LO Writer).
    bool fcitx5CommittedRewrite = false;
    /// Click-away policy when anchor fails (cursor moved to different line/context):
    /// false (default) — commit at new cursor position, then attempt rescue via
    ///   deleteSurroundingText if the preedit text is detected at the wrong place.
    /// true — do NOT commit; text is silently dropped so it never appears at the
    ///   wrong position. User must retype the word.
    bool fcitx5ClickAwayDropOnFail = false;
    /// M15: enable Smart Templates (parametric macro) via farolkey-template subprocess.
    bool enableSmartTemplates = true;
    /// M14.12: Free Layout user-defined shortcut + tone config.
    farolkey::core::FreeLayoutConfig freeLayout;
};

RuntimeConfig defaultConfig();
RuntimeConfig loadConfigFile(const std::string& path);
std::vector<std::string> validateConfig(const RuntimeConfig& config);

}  // namespace farolkey::config
