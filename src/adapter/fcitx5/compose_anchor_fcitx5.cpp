#include "cbakey/adapter/fcitx5/compose_anchor_fcitx5.h"

#include <fcitx/inputcontext.h>
#include <fcitx/surroundingtext.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>

#include "cbakey/adapter/fcitx5/compose_anchor.h"

namespace cbakey::adapter::fcitx5 {

namespace {

constexpr int kMaxCaretNudgeSteps = 256;

/// Count codepoints in a UTF-8 string (full string).
unsigned int countCpFull(const std::string& s) {
    return utf8CountCp(s, s.size());
}

/// Return true if \p pending appears immediately before the cursor in \p st
/// AND the surrounding text differs from \p snap_text (ruling out same-context
/// cases where the match is coincidental).
bool isPreeditAtWrongPlace(const fcitx::SurroundingText& st,
                            const std::string& pending,
                            const std::string& snap_text) {
    if (!st.isValid() || st.text().empty() || !st.selectedText().empty()) return false;
    if (st.text() == snap_text) return false;  // same context → not wrong place
    if (pending.empty()) return false;

    const std::string& txt = st.text();
    const std::size_t cur_byte = utf8ByteLen(txt, st.cursor());
    if (cur_byte < pending.size()) return false;

    return txt.substr(cur_byte - pending.size(), pending.size()) == pending;
}

}  // namespace

void refreshComposeAnchorForPreedit(fcitx::InputContext* ic,
                                    const std::string& preedit_utf8,
                                    ComposeAnchorSnapshot* out) {
    if (!out) return;
    *out = {};
    if (!ic || preedit_utf8.empty()) return;
    if (!ic->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText)) return;
    const auto& st = ic->surroundingText();
    if (!st.isValid() || !st.selectedText().empty()) return;
    out->valid = true;
    out->surrounding_text = st.text();
    out->cursor_u32 = st.cursor();
}

void commitPendingRespectingComposeAnchor(fcitx::InputContext* ic,
                                          const ComposeAnchorSnapshot& anchor,
                                          const std::string& pending,
                                          bool dropOnFail) {
    if (!ic || pending.empty()) return;

    const bool hasSurrounding =
        ic->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText);

    if (!hasSurrounding) {
        // No surrounding text capability — can't anchor.
        if (!dropOnFail) ic->commitString(pending);
        return;
    }

    // Request fresh surrounding text before computing nudge plan.
    // Helps with timing on some clients (Electron, Chrome) that update
    // their caret position asynchronously.
    ic->updateSurroundingText();
    const auto& st = ic->surroundingText();
    const auto plan = planCaretNudgeForAnchoredCommit(anchor, st.text(), st.cursor(),
                                                       st.selectedText().empty(),
                                                       kMaxCaretNudgeSteps);

    if (plan) {
        // Anchor succeeded — commit at the original preedit position.
        const fcitx::Key left{FcitxKey_Left, fcitx::KeyStates()};
        const fcitx::Key right{FcitxKey_Right, fcitx::KeyStates()};
        const fcitx::Key& first  = plan->left_first ? left  : right;
        const fcitx::Key& second = plan->left_first ? right : left;
        for (int i = 0; i < plan->steps; ++i) ic->forwardKey(first,  false, 0);
        ic->commitString(pending);
        for (int i = 0; i < plan->steps; ++i) ic->forwardKey(second, false, 0);
        return;
    }

    // ── Anchor failed (cursor moved to a different text context) ──────────────

    if (dropOnFail) {
        // Policy: drop — never commit at the wrong position.
        // The user will see the preedit disappear without landing anywhere;
        // they can retype the word at the new cursor position.
        return;
    }

    // Policy: commit at new position, then attempt rescue via deleteSurroundingText.
    ic->commitString(pending);

    // Best-effort rescue: detect whether the preedit text landed immediately before
    // the new cursor in a different text context and, if so, delete it there.
    //
    // Reliability:
    //   X11   — surrounding text typically updates synchronously → rescue fires correctly.
    //   Wayland — update is async; the check may see stale data on the first attempt.
    //             For apps that flush surrounding text before the next event (LibreOffice
    //             Writer, some GTK apps), rescue will still fire.
    if (anchor.valid) {
        ic->updateSurroundingText();
        const auto& st2 = ic->surroundingText();
        if (isPreeditAtWrongPlace(st2, pending, anchor.surrounding_text)) {
            const int cp_len = static_cast<int>(countCpFull(pending));
            ic->deleteSurroundingText(-cp_len, 0);
            // Text removed from wrong position. It cannot be re-inserted at the old
            // position without absolute document coordinates (Fcitx5 API limitation).
            // The user must retype — but the text no longer appears at a random place.
        }
    }
}

}  // namespace cbakey::adapter::fcitx5
