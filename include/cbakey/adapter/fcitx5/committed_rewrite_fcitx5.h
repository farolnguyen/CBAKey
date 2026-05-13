#pragma once

#include "cbakey/config/config.h"
#include "cbakey/core/types.h"

namespace fcitx {
class InputContext;
}

namespace cbakey::adapter::fcitx5 {

/// M6.3a prototype (only when `RuntimeConfig::fcitx5CommittedRewrite` is true): if surrounding text
/// + caret allow rewriting the syllable immediately left of the caret with \p event, applies
/// delete+commit and returns true (key fully handled).
bool tryApplyCommittedSyllableRewrite(fcitx::InputContext* ic,
                                      const cbakey::config::RuntimeConfig& config,
                                      const cbakey::core::KeyEvent& event);

}  // namespace cbakey::adapter::fcitx5
