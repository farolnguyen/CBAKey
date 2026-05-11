#pragma once

#include <string>
#include <vector>

#include "cbakey/config/config.h"
#include "cbakey/core/types.h"

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

private:
    bool isToggleHotkey(const KeyEvent& event) const;
    ProcessResult processVietnameseKey(const KeyEvent& event);
    ProcessResult processEnglishKey(const KeyEvent& event);
    static bool isAsciiSeparatorCommit(char ch);

    cbakey::config::RuntimeConfig config_;
    InputMode mode_ = InputMode::Vietnamese;
    std::string preeditBuffer_;
    std::vector<std::string> preeditHistory_;
};

}  // namespace cbakey::core
