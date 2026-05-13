#include "cbakey/adapter/fcitx5/compose_anchor_fcitx5.h"

#include <fcitx/inputcontext.h>
#include <fcitx/surroundingtext.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>

namespace cbakey::adapter::fcitx5 {

namespace {

constexpr int kMaxCaretNudgeSteps = 256;

}  // namespace

void refreshComposeAnchorForPreedit(fcitx::InputContext* ic,
                                    const std::string& preedit_utf8,
                                    ComposeAnchorSnapshot* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (!ic || preedit_utf8.empty()) {
        return;
    }
    if (!ic->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText)) {
        return;
    }
    const auto& st = ic->surroundingText();
    if (!st.isValid() || !st.selectedText().empty()) {
        return;
    }
    out->valid = true;
    out->surrounding_text = st.text();
    out->cursor_u32 = st.cursor();
}

void commitPendingRespectingComposeAnchor(fcitx::InputContext* ic,
                                          const ComposeAnchorSnapshot& anchor,
                                          const std::string& pending) {
    if (!ic || pending.empty()) {
        return;
    }
    if (!ic->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText)) {
        ic->commitString(pending);
        return;
    }
    const auto& st = ic->surroundingText();
    const auto plan = planCaretNudgeForAnchoredCommit(anchor, st.text(), st.cursor(),
                                                       st.selectedText().empty(), kMaxCaretNudgeSteps);
    if (plan) {
        const fcitx::Key left{FcitxKey_Left, fcitx::KeyStates()};
        const fcitx::Key right{FcitxKey_Right, fcitx::KeyStates()};
        const fcitx::Key& first = plan->left_first ? left : right;
        const fcitx::Key& second = plan->left_first ? right : left;
        for (int i = 0; i < plan->steps; ++i) {
            ic->forwardKey(first, false, 0);
        }
        ic->commitString(pending);
        for (int i = 0; i < plan->steps; ++i) {
            ic->forwardKey(second, false, 0);
        }
        return;
    }
    ic->commitString(pending);
}

}  // namespace cbakey::adapter::fcitx5
