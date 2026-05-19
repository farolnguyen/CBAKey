#include <cassert>

#include "farolkey/adapter/fcitx5/compose_anchor.h"

using farolkey::adapter::fcitx5::ComposeAnchorSnapshot;
using farolkey::adapter::fcitx5::planCaretNudgeForAnchoredCommit;

int main() {
    assert(!planCaretNudgeForAnchoredCommit(ComposeAnchorSnapshot{}, "x", 1, true, 256).has_value());

    ComposeAnchorSnapshot snap{.valid = true,
                               .surrounding_text = "hello",
                               .cursor_u32 = 3};
    assert(!planCaretNudgeForAnchoredCommit(snap, "hello", 3, true, 256).has_value());
    const auto backOne = planCaretNudgeForAnchoredCommit(snap, "hello", 2, true, 256);
    assert(backOne.has_value());
    assert(backOne->steps == 1);
    assert(!backOne->left_first);
    assert(!planCaretNudgeForAnchoredCommit(snap, "hellx", 5, true, 256).has_value());
    assert(!planCaretNudgeForAnchoredCommit(snap, "hello", 5, false, 256).has_value());

    const auto forward = planCaretNudgeForAnchoredCommit(snap, "hello", 5, true, 256);
    assert(forward.has_value());
    assert(forward->steps == 2);
    assert(forward->left_first);

    assert(!planCaretNudgeForAnchoredCommit(snap, "hello", 300, true, 256).has_value());

    const auto back = planCaretNudgeForAnchoredCommit(snap, "hello", 1, true, 256);
    assert(back.has_value());
    assert(back->steps == 2);
    assert(!back->left_first);

    return 0;
}
