#pragma once

#include <string>
#include <vector>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"
#include "cbakey/core/types.h"

namespace cbakey::adapter::fcitx5 {

class Bridge {
public:
    explicit Bridge(const cbakey::config::RuntimeConfig& config);

    cbakey::core::ProcessResult handleKey(const cbakey::core::KeyEvent& event);
    const std::string& preedit() const;
    const cbakey::config::RuntimeConfig& config() const;
    std::vector<std::string> drainCommitted();
    cbakey::core::InputMode inputMode() const;
    void reset();
    /// Clears composition and returns text that should be committed (IM deactivate / flush).
    std::string takeCompositionForCommit();

    /// M13: propagate password-field state to the engine (disables abbreviation expansion).
    void setPasswordField(bool isPassword);

    /// M13: look up an En/Both abbreviation trigger (for English-mode surrounding-text rewrite).
    const cbakey::core::UserDictEntry* lookupEnglishAbbrev(const std::string& trigger) const;

private:
    cbakey::config::RuntimeConfig config_;
    cbakey::core::Engine engine_;
    std::string preedit_;
    std::vector<std::string> committed_;
};

Bridge createBridgeFromConfigFile(const std::string& configPath);

}  // namespace cbakey::adapter::fcitx5
