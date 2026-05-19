#pragma once

#include <string>

#include "farolkey/adapter/fcitx5/compose_anchor.h"

namespace fcitx {
class InputContext;
}

namespace farolkey::adapter::fcitx5 {

/// If \p preedit_utf8 is non-empty and surrounding text is available, refresh \p out.
/// Otherwise clears \p out.
void refreshComposeAnchorForPreedit(fcitx::InputContext* ic,
                                    const std::string& preedit_utf8,
                                    ComposeAnchorSnapshot* out);

/// Commits \p pending while trying to keep insertion at the preedit anchor.
///
/// When the anchor succeeds (cursor moved within the same text context):
///   sends synthetic Left/Right arrows around commitString to land at the snap position.
///
/// When the anchor fails (cursor moved to a different line or context):
///   - dropOnFail=false (default): commits at new cursor position, then attempts a
///     best-effort rescue via deleteSurroundingText if the preedit text is detected
///     immediately before the new cursor (works reliably on X11; best-effort on Wayland).
///   - dropOnFail=true: does NOT commit — text is silently dropped so it never appears
///     at the wrong position. The user must retype.
void commitPendingRespectingComposeAnchor(fcitx::InputContext* ic,
                                           const ComposeAnchorSnapshot& anchor,
                                           const std::string& pending,
                                           bool dropOnFail = false);

}  // namespace farolkey::adapter::fcitx5
