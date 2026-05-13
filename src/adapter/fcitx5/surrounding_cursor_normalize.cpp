#include "cbakey/adapter/fcitx5/surrounding_cursor_normalize.h"

#include <algorithm>
#include <cstddef>
#include <string>

namespace cbakey::adapter::fcitx5 {
namespace {

bool decodeOneUtf8(const std::string& s, std::size_t* i, char32_t* out) {
    if (*i >= s.size()) {
        return false;
    }
    const unsigned char c0 = static_cast<unsigned char>(s[*i]);
    if ((c0 & 0x80U) == 0U) {
        *out = static_cast<char32_t>(c0);
        ++(*i);
        return true;
    }
    if (*i + 1 < s.size() && (c0 & 0xE0U) == 0xC0U) {
        const char32_t cp =
            ((static_cast<char32_t>(c0) & 0x1FU) << 6) |
            (static_cast<unsigned char>(s[*i + 1]) & 0x3FU);
        *out = cp;
        *i += 2;
        return true;
    }
    if (*i + 2 < s.size() && (c0 & 0xF0U) == 0xE0U) {
        const char32_t cp =
            ((static_cast<char32_t>(c0) & 0x0FU) << 12) |
            ((static_cast<unsigned char>(s[*i + 1]) & 0x3FU) << 6) |
            (static_cast<unsigned char>(s[*i + 2]) & 0x3FU);
        *out = cp;
        *i += 3;
        return true;
    }
    if (*i + 3 < s.size() && (c0 & 0xF8U) == 0xF0U) {
        const char32_t cp =
            ((static_cast<char32_t>(c0) & 0x07U) << 18) |
            ((static_cast<unsigned char>(s[*i + 1]) & 0x3FU) << 12) |
            ((static_cast<unsigned char>(s[*i + 2]) & 0x3FU) << 6) |
            (static_cast<unsigned char>(s[*i + 3]) & 0x3FU);
        *out = cp;
        *i += 4;
        return true;
    }
    ++(*i);
    *out = U'?';
    return true;
}

unsigned int countUtf8Scalars(const std::string& s) {
    std::size_t i = 0;
    unsigned n = 0;
    while (i < s.size()) {
        char32_t ch = 0;
        decodeOneUtf8(s, &i, &ch);
        ++n;
    }
    return n;
}

/// Number of Unicode scalars whose UTF-8 encoding lies entirely in `s[0, byte_limit)`.
unsigned int utf8ByteOffsetToScalarIndex(const std::string& s, unsigned int byte_limit) {
    const std::size_t limit = std::min<std::size_t>(byte_limit, s.size());
    std::size_t pos = 0;
    unsigned scalars = 0;
    while (pos < s.size() && pos < limit) {
        char32_t ch = 0;
        decodeOneUtf8(s, &pos, &ch);
        if (pos > limit) {
            return scalars;
        }
        (void)ch;
        ++scalars;
    }
    return scalars;
}

}  // namespace

unsigned int normalizeSurroundingCursorToCodepointIndex(const std::string& utf8_text,
                                                        unsigned int cursor_raw) {
    const unsigned int n = countUtf8Scalars(utf8_text);
    if (cursor_raw <= n) {
        return cursor_raw;
    }
    if (cursor_raw > utf8_text.size()) {
        return n;
    }
    return utf8ByteOffsetToScalarIndex(utf8_text, cursor_raw);
}

}  // namespace cbakey::adapter::fcitx5
