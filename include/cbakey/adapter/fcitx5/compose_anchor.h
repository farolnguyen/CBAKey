#pragma once

#include <optional>
#include <string>

namespace cbakey::adapter::fcitx5 {

/// Snapshot of client surrounding text taken while a non-empty preedit is active.
struct ComposeAnchorSnapshot {
    bool valid = false;
    std::string surrounding_text;
    unsigned int cursor_u32 = 0;
};

struct CaretNudgePlan {
    int steps = 0;
    bool left_first = true;  // true → Left×N before commit, Right×N after
};

// ── UTF-8 helpers ────────────────────────────────────────────────────────────

/// Byte offset of the N-th codepoint boundary in \p utf8 (clamped to string length).
inline std::size_t utf8ByteLen(const std::string& utf8, std::size_t n_codepoints) {
    std::size_t b = 0, cp = 0;
    while (b < utf8.size() && cp < n_codepoints) {
        const unsigned char c = static_cast<unsigned char>(utf8[b]);
        b += (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
        ++cp;
    }
    return b;
}

/// Count codepoints in \p utf8 up to byte offset \p byte_limit.
inline unsigned int utf8CountCp(const std::string& utf8, std::size_t byte_limit) {
    unsigned int cp = 0;
    std::size_t b = 0;
    while (b < byte_limit && b < utf8.size()) {
        const unsigned char c = static_cast<unsigned char>(utf8[b]);
        b += (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
        ++cp;
    }
    return cp;
}

/// Step one UTF-8 codepoint backward from byte position \p b in \p s.
inline std::size_t utf8StepBack(const std::string& s, std::size_t b) {
    if (b == 0) return 0;
    --b;
    while (b > 0 && (static_cast<unsigned char>(s[b]) & 0xC0) == 0x80) --b;
    return b;
}

// ── Strategy 1: Exact match ───────────────────────────────────────────────────

inline std::optional<CaretNudgePlan> planCaretNudgeExact(const ComposeAnchorSnapshot& snap,
                                                          const std::string& now_text,
                                                          unsigned int now_cursor,
                                                          bool now_selection_empty,
                                                          int max_nudge) {
    if (!snap.valid || !now_selection_empty) return std::nullopt;
    if (now_text != snap.surrounding_text) return std::nullopt;
    const long delta = static_cast<long>(now_cursor) - static_cast<long>(snap.cursor_u32);
    if (delta == 0) return std::nullopt;
    const long abs_d = delta < 0 ? -delta : delta;
    if (abs_d > static_cast<long>(max_nudge)) return std::nullopt;
    return CaretNudgePlan{.steps = static_cast<int>(abs_d), .left_first = (delta > 0)};
}

// ── Strategy 2: Relaxed — centered context (before + after cursor) ────────────

inline std::optional<CaretNudgePlan> planCaretNudgeCentered(const ComposeAnchorSnapshot& snap,
                                                              const std::string& now_text,
                                                              unsigned int now_cursor,
                                                              bool now_selection_empty,
                                                              int max_nudge,
                                                              int ctx_half = 48) {
    if (!snap.valid || !now_selection_empty || now_text.empty()) return std::nullopt;
    if (snap.surrounding_text.empty()) return std::nullopt;

    const std::size_t snap_cur_byte = utf8ByteLen(snap.surrounding_text, snap.cursor_u32);

    // Walk backward ctx_half codepoints
    std::size_t ctx_start = snap_cur_byte;
    for (int i = 0; i < ctx_half && ctx_start > 0; ++i)
        ctx_start = utf8StepBack(snap.surrounding_text, ctx_start);

    // Walk forward ctx_half codepoints
    std::size_t ctx_end = snap_cur_byte;
    for (int i = 0; i < ctx_half && ctx_end < snap.surrounding_text.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(snap.surrounding_text[ctx_end]);
        ctx_end += (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
    }
    ctx_end = std::min(ctx_end, snap.surrounding_text.size());

    const std::string ctx = snap.surrounding_text.substr(ctx_start, ctx_end - ctx_start);
    if (ctx.empty()) return std::nullopt;

    const std::size_t found = now_text.find(ctx);
    if (found == std::string::npos) return std::nullopt;
    if (now_text.find(ctx, found + 1) != std::string::npos) return std::nullopt;  // not unique

    // Snap cursor in now_text = found + (snap_cur_byte - ctx_start) bytes from start
    const std::size_t snap_in_now_byte = found + (snap_cur_byte - ctx_start);
    const unsigned int snap_in_now_cp = utf8CountCp(now_text, snap_in_now_byte);

    const long delta = static_cast<long>(now_cursor) - static_cast<long>(snap_in_now_cp);
    if (delta == 0) return std::nullopt;
    const long abs_d = delta < 0 ? -delta : delta;
    if (abs_d > static_cast<long>(max_nudge)) return std::nullopt;
    return CaretNudgePlan{.steps = static_cast<int>(abs_d), .left_first = (delta > 0)};
}

// ── Strategy 3: Left-only context (text BEFORE snap cursor) ──────────────────
//
// Searches for the text immediately preceding the snap cursor inside now_text.
// More stable than centered: the text to the LEFT of the cursor hasn't been
// modified by preedit composition, so it's a cleaner anchor.
// Works better when apps expose a small surrounding-text window (e.g. VSCode,
// LibreOffice Writer) because we only need the LEFT side to overlap.

inline std::optional<CaretNudgePlan> planCaretNudgeLeftOnly(const ComposeAnchorSnapshot& snap,
                                                              const std::string& now_text,
                                                              unsigned int now_cursor,
                                                              bool now_selection_empty,
                                                              int max_nudge,
                                                              int left_ctx_size = 64) {
    if (!snap.valid || !now_selection_empty || now_text.empty()) return std::nullopt;
    if (snap.surrounding_text.empty()) return std::nullopt;

    const std::size_t snap_cur_byte = utf8ByteLen(snap.surrounding_text, snap.cursor_u32);
    if (snap_cur_byte == 0) return std::nullopt;  // nothing to the left

    // Walk backward left_ctx_size codepoints from snap cursor
    std::size_t left_start = snap_cur_byte;
    for (int i = 0; i < left_ctx_size && left_start > 0; ++i)
        left_start = utf8StepBack(snap.surrounding_text, left_start);

    const std::string left_ctx = snap.surrounding_text.substr(left_start,
                                                               snap_cur_byte - left_start);
    if (left_ctx.size() < 4) return std::nullopt;  // too short → too many false positives

    // Find left_ctx in now_text; old cursor = position right AFTER left_ctx
    const std::size_t found = now_text.find(left_ctx);
    if (found == std::string::npos) return std::nullopt;
    if (now_text.find(left_ctx, found + 1) != std::string::npos) return std::nullopt;  // ambiguous

    const std::size_t old_cur_byte_in_now = found + left_ctx.size();
    const unsigned int old_cur_cp = utf8CountCp(now_text, old_cur_byte_in_now);

    const long delta = static_cast<long>(now_cursor) - static_cast<long>(old_cur_cp);
    if (delta == 0) return std::nullopt;
    const long abs_d = delta < 0 ? -delta : delta;
    if (abs_d > static_cast<long>(max_nudge)) return std::nullopt;
    return CaretNudgePlan{.steps = static_cast<int>(abs_d), .left_first = (delta > 0)};
}

// ── Unified entry point ───────────────────────────────────────────────────────
//
// Tries strategies in order: exact → centered (48cp) → left-only (64cp).
// Returns nullopt when all strategies fail (cursor moved too far or no text overlap).

inline std::optional<CaretNudgePlan> planCaretNudgeForAnchoredCommit(const ComposeAnchorSnapshot& snap,
                                                                      const std::string& now_text,
                                                                      unsigned int now_cursor,
                                                                      bool now_selection_empty,
                                                                      int max_nudge) {
    if (const auto p = planCaretNudgeExact(snap, now_text, now_cursor, now_selection_empty, max_nudge))
        return p;
    if (const auto p = planCaretNudgeCentered(snap, now_text, now_cursor, now_selection_empty, max_nudge))
        return p;
    return planCaretNudgeLeftOnly(snap, now_text, now_cursor, now_selection_empty, max_nudge);
}

}  // namespace cbakey::adapter::fcitx5
