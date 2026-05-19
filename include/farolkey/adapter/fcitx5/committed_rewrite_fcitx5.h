#pragma once

#include "farolkey/config/config.h"
#include "farolkey/core/types.h"

namespace fcitx {
class InputContext;
}

namespace farolkey::adapter::fcitx5 {

/// M6.3a: rewrite the syllable immediately left of the caret with \p event.
/// Primary path: uses SurroundingText API (works in GTK/Qt/Teams apps).
/// Fallback path: when app does not expose SurroundingText (Chrome/Electron
///   contenteditable), uses \p fallbackToken — the last word committed by the
///   IME — emitting N Backspaces then committing the rewritten form.
///   Safe only when cursor is right after the committed token.
bool tryApplyCommittedSyllableRewrite(fcitx::InputContext* ic,
                                      const farolkey::config::RuntimeConfig& config,
                                      const farolkey::core::KeyEvent& event,
                                      const std::string& fallbackToken = {});

}  // namespace farolkey::adapter::fcitx5
