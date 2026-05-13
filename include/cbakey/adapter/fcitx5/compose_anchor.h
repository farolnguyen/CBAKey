#pragma once

#include <optional>
#include <string>

namespace cbakey::adapter::fcitx5 {

/// Snapshot of client surrounding text taken while a non-empty preedit is active.
/// Used on reset/deactivate to avoid committing unfinished composition at a caret
/// position that already moved (common client ordering bug).
struct ComposeAnchorSnapshot {
    bool valid = false;
    std::string surrounding_text;
    unsigned int cursor_u32 = 0;
};

/// Describes synthetic arrow presses around \c commitString so insertion stays at the
/// preedit anchor when surrounding text is unchanged and only the caret moved.
struct CaretNudgePlan {
    int steps = 0;
    /// When true: Left×steps before commit, then Right×steps after (caret moved forward).
    /// When false: Right×steps before commit, then Left×steps after (caret moved backward).
    bool left_first = true;
};

/// Returns a nudge plan when surrounding buffer is unchanged, there is no selection, and the
/// caret moved by at most \p max_nudge positions (either direction).
inline std::optional<CaretNudgePlan> planCaretNudgeForAnchoredCommit(const ComposeAnchorSnapshot& snap,
                                                                     const std::string& now_text,
                                                                     unsigned int now_cursor,
                                                                     bool now_selection_empty,
                                                                     int max_nudge) {
    if (!snap.valid || !now_selection_empty) {
        return std::nullopt;
    }
    if (now_text != snap.surrounding_text) {
        return std::nullopt;
    }
    const long delta = static_cast<long>(now_cursor) - static_cast<long>(snap.cursor_u32);
    if (delta == 0) {
        return std::nullopt;
    }
    const long steps_l = delta >= 0 ? delta : -delta;
    if (steps_l > static_cast<long>(max_nudge)) {
        return std::nullopt;
    }
    CaretNudgePlan plan;
    plan.steps = static_cast<int>(steps_l);
    plan.left_first = (delta > 0);
    return plan;
}

}  // namespace cbakey::adapter::fcitx5
