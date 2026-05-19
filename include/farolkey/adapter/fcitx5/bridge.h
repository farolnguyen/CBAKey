#pragma once

#include <string>
#include <vector>

#include "farolkey/config/config.h"
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"

namespace farolkey::adapter::fcitx5 {

class Bridge {
public:
    explicit Bridge(const farolkey::config::RuntimeConfig& config);

    farolkey::core::ProcessResult handleKey(const farolkey::core::KeyEvent& event);
    const std::string& preedit() const;
    const farolkey::config::RuntimeConfig& config() const;
    std::vector<std::string> drainCommitted();
    farolkey::core::InputMode inputMode() const;
    void setInputMode(farolkey::core::InputMode mode);
    void reset();
    /// Reload the bridge with a new RuntimeConfig (called when user changes settings).
    void reloadConfig(const farolkey::config::RuntimeConfig& config);
    /// Clears composition and returns text that should be committed (IM deactivate / flush).
    std::string takeCompositionForCommit();

    /// M13: propagate password-field state to the engine (disables abbreviation expansion).
    void setPasswordField(bool isPassword);

    /// M13: look up an En/Both abbreviation trigger (for English-mode surrounding-text rewrite).
    const farolkey::core::UserDictEntry* lookupEnglishAbbrev(const std::string& trigger) const;

    /// Returns the last Vietnamese word committed by the IME (without trailing space/punct).
    /// Used as fallback for committed-rewrite when SurroundingText is not available.
    const std::string& lastCommittedViWord() const { return lastCommittedViWord_; }

private:
    farolkey::config::RuntimeConfig config_;
    farolkey::core::Engine engine_;
    std::string preedit_;
    std::vector<std::string> committed_;
    std::string lastCommittedViWord_;   // last VI-mode word committed (no suffix)
};

Bridge createBridgeFromConfigFile(const std::string& configPath);

}  // namespace farolkey::adapter::fcitx5
