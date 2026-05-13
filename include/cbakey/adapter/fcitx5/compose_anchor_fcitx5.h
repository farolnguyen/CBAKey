#pragma once

#include <string>

#include "cbakey/adapter/fcitx5/compose_anchor.h"

namespace fcitx {
class InputContext;
}

namespace cbakey::adapter::fcitx5 {

/// If \p preedit_utf8 is non-empty and surrounding text is available, refresh \p out.
/// Otherwise clears \p out.
void refreshComposeAnchorForPreedit(fcitx::InputContext* ic,
                                    const std::string& preedit_utf8,
                                    ComposeAnchorSnapshot* out);

/// Commits \p pending, optionally sending synthetic Left/Right to keep insertion at the
/// preedit anchor when surrounding text is unchanged and the caret moved (forward or backward).
void commitPendingRespectingComposeAnchor(fcitx::InputContext* ic,
                                           const ComposeAnchorSnapshot& anchor,
                                           const std::string& pending);

}  // namespace cbakey::adapter::fcitx5
