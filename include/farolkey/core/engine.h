#pragma once

#include <optional>
#include <string>
#include <vector>

#include "farolkey/config/config.h"
#include "farolkey/core/types.h"
#include "farolkey/core/user_dict.h"

namespace farolkey::core {

class Engine {
public:
    explicit Engine(farolkey::config::RuntimeConfig config);

    ProcessResult processKey(const KeyEvent& event);
    void setInputMode(InputMode mode);
    InputMode inputMode() const;
    void clearState();

    /// M13: disable abbreviation expansion when focused widget is a password/sensitive field.
    void setPasswordField(bool isPassword);
    bool isPasswordField() const;

    /// M13: look up a trigger for En/Both-mode expansion (used by the adapter's
    /// surrounding-text rewrite path in English mode). Returns nullptr if not found,
    /// dict is disabled, or password field is active.
    const UserDictEntry* lookupEnglishAbbrev(const std::string& trigger) const;
    /// Returns current composition text and clears engine state (for IM switch / flush).
    std::string takeCompositionForCommit();

    /// M6.3a: load UTF-8 as the active composition (no simulated keypresses). Used for
    /// committed-syllable rewrite; clears history/repeat state first.
    void seedPreeditForCommittedRewrite(std::string utf8);

    /// M6.3a prototype: if \p token_utf8 is one confident syllable and \p event applies a
    /// rewrite-only transform, returns the new UTF-8 syllable. Otherwise nullopt.
    static std::optional<std::string> tryRewriteCommittedSyllable(const farolkey::config::RuntimeConfig& config,
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

    farolkey::config::RuntimeConfig config_;
    UserDict userDict_;
    InputMode mode_ = InputMode::Vietnamese;
    std::string preeditBuffer_;
    std::vector<std::string> preeditHistory_;
    RepeatTransformState repeatTransformState_;
    bool pendingLiteralEscape_ = false;
    bool passwordField_        = false;
};

}  // namespace farolkey::core
