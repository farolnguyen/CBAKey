#include <cassert>
#include <string>

#include "cbakey/adapter/fcitx5/surrounding_cursor_normalize.h"

int main() {
    using cbakey::adapter::fcitx5::normalizeSurroundingCursorToCodepointIndex;

    const std::string ascii = "hello";
    assert(normalizeSurroundingCursorToCodepointIndex(ascii, 5) == 5);
    assert(normalizeSurroundingCursorToCodepointIndex(ascii, 3) == 3);

    // "à" = U+00E0, UTF-8 C3 A0 (2 bytes, 1 scalar). Electron/VSCode often reports byte offset 2.
    const std::string a_grave = "à";
    assert(normalizeSurroundingCursorToCodepointIndex(a_grave, 1) == 1);
    assert(normalizeSurroundingCursorToCodepointIndex(a_grave, 2) == 1);

    const std::string mixed = "aàb";
    assert(normalizeSurroundingCursorToCodepointIndex(mixed, 3) == 3);
    assert(normalizeSurroundingCursorToCodepointIndex(mixed, 4) == 3);

    const std::string empty;
    assert(normalizeSurroundingCursorToCodepointIndex(empty, 0) == 0);
    assert(normalizeSurroundingCursorToCodepointIndex(empty, 99) == 0);

    return 0;
}
