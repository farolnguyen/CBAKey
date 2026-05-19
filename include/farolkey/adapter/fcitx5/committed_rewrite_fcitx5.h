#pragma once

#include "farolkey/config/config.h"
#include "farolkey/core/types.h"

namespace fcitx {
class InputContext;
}

namespace farolkey::adapter::fcitx5 {

/// M6.3a prototype (only when `RuntimeConfig::fcitx5CommittedRewrite` is true): if surrounding text
/// + caret allow rewriting the syllable immediately left of the caret with \p event, applies
/// delete+commit and returns true (key fully handled).
bool tryApplyCommittedSyllableRewrite(fcitx::InputContext* ic,
                                      const farolkey::config::RuntimeConfig& config,
                                      const farolkey::core::KeyEvent& event);

}  // namespace farolkey::adapter::fcitx5
