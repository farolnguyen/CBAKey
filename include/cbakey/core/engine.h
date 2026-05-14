#pragma once

#include <optional>
#include <string>
#include <vector>

#include "cbakey/config/config.h"
#include "cbakey/core/types.h"
#include "cbakey/core/user_dict.h"

namespace cbakey::core {

class Engine {
public:
    explicit Engine(cbakey::config::RuntimeConfig config);

    ProcessResult processKey(const KeyEvent& event);
    void setInputMode(InputMode mode);
    InputMode inputMode() const;
    void clearState();
    /// Returns current composition text and clears engine state (for IM switch / flush).
    std::string takeCompositionForCommit();

    /// M6.3a: load UTF-8 as the active composition (no simulated keypresses). Used for
    /// committed-syllable rewrite; clears history/repeat state first.
    void seedPreeditForCommittedRewrite(std::string utf8);

    /// M6.3a prototype: if \p token_utf8 is one confident syllable and \p event applies a
    /// rewrite-only transform, returns the new UTF-8 syllable. Otherwise nullopt.
    static std::optional<std::string> tryRewriteCommittedSyllable(const cbakey::config::RuntimeConfig& config,
                                                                  const std::string& token_utf8,
                                                                  const KeyEvent& event);

private:
    struct RepeatTransformState {
        bool active = false;
        char key = '\0';
        std::string buffer_before;
    };

    void clearRepeatTransformState();
    void clearPendingLiteralEscape();
    bool isToggleHotkey(const KeyEvent& event) const;
    ProcessResult processVietnameseKey(const KeyEvent& event);
    ProcessResult processEnglishKey(const KeyEvent& event);
    static bool isAsciiSeparatorCommit(char ch);

    cbakey::config::RuntimeConfig config_;
    UserDict userDict_;
    InputMode mode_ = InputMode::Vietnamese;
    std::string preeditBuffer_;
    std::vector<std::string> preeditHistory_;
    RepeatTransformState repeatTransformState_;
    bool pendingLiteralEscape_ = false;
};

}  // namespace cbakey::core
