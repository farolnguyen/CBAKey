#include "farolkey/adapter/fcitx5/bridge.h"

namespace farolkey::adapter::fcitx5 {

Bridge::Bridge(const farolkey::config::RuntimeConfig& config) : config_(config), engine_(config) {}

farolkey::core::ProcessResult Bridge::handleKey(const farolkey::core::KeyEvent& event) {
    const auto result = engine_.processKey(event);
    preedit_ = result.preedit;
    if (!result.commit.empty()) {
        committed_.push_back(result.commit);
        // Track the last committed Vietnamese word (strip trailing space/punct suffix)
        // for use as a fallback in committed-rewrite when SurroundingText is absent.
        if (!result.smartTemplateExpansion) {
            std::string word = result.commit;
            while (!word.empty() &&
                   (word.back() == ' ' || word.back() == '\n' || word.back() == '\t')) {
                word.pop_back();
            }
            if (!word.empty()) {
                lastCommittedViWord_ = std::move(word);
            }
        }
    } else if (!result.preedit.empty()) {
        // New preedit started — old lastCommittedViWord still valid until next commit.
    } else if (result.consumed && result.commit.empty() && result.preedit.empty()) {
        // A non-commit key consumed (e.g. Backspace on empty buffer) — invalidate word.
        lastCommittedViWord_.clear();
    }
    return result;
}

const std::string& Bridge::preedit() const {
    return preedit_;
}

const farolkey::config::RuntimeConfig& Bridge::config() const {
    return config_;
}

std::vector<std::string> Bridge::drainCommitted() {
    auto out = committed_;
    committed_.clear();
    return out;
}

farolkey::core::InputMode Bridge::inputMode() const {
    return engine_.inputMode();
}

void Bridge::setInputMode(farolkey::core::InputMode mode) {
    engine_.setInputMode(mode);
}

void Bridge::reloadConfig(const farolkey::config::RuntimeConfig& config) {
    config_ = config;
    engine_ = farolkey::core::Engine(config_);
    preedit_.clear();
    committed_.clear();
}

void Bridge::reset() {
    engine_.clearState();
    preedit_.clear();
    committed_.clear();
}

std::string Bridge::takeCompositionForCommit() {
    std::string out = engine_.takeCompositionForCommit();
    preedit_.clear();
    return out;
}

void Bridge::setPasswordField(bool isPassword) {
    engine_.setPasswordField(isPassword);
}

const farolkey::core::UserDictEntry* Bridge::lookupEnglishAbbrev(const std::string& trigger) const {
    return engine_.lookupEnglishAbbrev(trigger);
}

Bridge createBridgeFromConfigFile(const std::string& configPath) {
    const auto config = farolkey::config::loadConfigFile(configPath);
    return Bridge(config);
}

}  // namespace farolkey::adapter::fcitx5
