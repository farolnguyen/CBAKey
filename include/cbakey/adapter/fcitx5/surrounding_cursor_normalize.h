#pragma once

#include <string>

namespace cbakey::adapter::fcitx5 {

/// Map a client-reported SurroundingText cursor value to a **Unicode scalar** index in
/// `[0, num_scalars]` suitable for logic that indexes decoded UTF-8 (same convention as Fcitx
/// "character" when the client is well-behaved).
///
/// Some Electron/VSCode paths report a **UTF-8 byte offset** that exceeds the scalar count
/// (e.g. caret after `"à"` reports `2` bytes but `1` scalar). In that ambiguous band
/// `(num_scalars, text.size()]`, interpret \p cursor_raw as a UTF-8 byte offset into \p utf8_text.
/// If \p cursor_raw is larger than the UTF-8 length, clamp to the end (last resort for broken
/// clients).
unsigned int normalizeSurroundingCursorToCodepointIndex(const std::string& utf8_text,
                                                        unsigned int cursor_raw);

}  // namespace cbakey::adapter::fcitx5
