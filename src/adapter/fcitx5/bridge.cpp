#include "cbakey/adapter/fcitx5/bridge.h"

namespace cbakey::adapter::fcitx5 {

Bridge::Bridge(const cbakey::config::RuntimeConfig& config) : engine_(config) {}

cbakey::core::ProcessResult Bridge::handleKey(const cbakey::core::KeyEvent& event) {
    const auto result = engine_.processKey(event);
    preedit_ = result.preedit;
    if (!result.commit.empty()) {
        committed_.push_back(result.commit);
    }
    return result;
}

const std::string& Bridge::preedit() const {
    return preedit_;
}

std::vector<std::string> Bridge::drainCommitted() {
    auto out = committed_;
    committed_.clear();
    return out;
}

cbakey::core::InputMode Bridge::inputMode() const {
    return engine_.inputMode();
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

Bridge createBridgeFromConfigFile(const std::string& configPath) {
    const auto config = cbakey::config::loadConfigFile(configPath);
    return Bridge(config);
}

}  // namespace cbakey::adapter::fcitx5
