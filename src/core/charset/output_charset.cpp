#include "farolkey/core/charset/output_charset.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace farolkey::core::charset {

// ── UTF-8 helpers ──────────────────────────────────────────────────────────────

// Decode one UTF-8 codepoint from [it, end). Advances it.
// Returns U+FFFD on invalid sequence.
static char32_t nextCodepoint(const char*& it, const char* end) {
    const uint8_t b0 = static_cast<uint8_t>(*it++);
    if (b0 < 0x80) return b0;
    if (b0 < 0xC0) return 0xFFFD;  // continuation byte out of place

    int extra;
    char32_t cp;
    if      (b0 < 0xE0) { extra = 1; cp = b0 & 0x1F; }
    else if (b0 < 0xF0) { extra = 2; cp = b0 & 0x0F; }
    else                { extra = 3; cp = b0 & 0x07; }

    for (int i = 0; i < extra; ++i) {
        if (it == end || (static_cast<uint8_t>(*it) & 0xC0) != 0x80)
            return 0xFFFD;
        cp = (cp << 6) | (static_cast<uint8_t>(*it++) & 0x3F);
    }
    return cp;
}


// ── Charset tables (populated per-charset in M22.2/M22.3/M22.4) ───────────────

// Forward-declared table accessors — each charset file defines its own.
const std::unordered_map<char32_t, uint8_t>& tcvn3Table();
const std::unordered_map<char32_t, uint8_t>& cp1258Table();
const std::unordered_map<char32_t, uint8_t>& visciiTable();

// ── Core conversion ────────────────────────────────────────────────────────────

static std::string convertWithTable(
        const std::string& utf8text,
        const std::unordered_map<char32_t, uint8_t>& table) {
    std::string out;
    out.reserve(utf8text.size());
    const char* it  = utf8text.data();
    const char* end = it + utf8text.size();
    while (it != end) {
        const char* before = it;
        char32_t cp = nextCodepoint(it, end);
        if (cp < 0x80) {
            // ASCII: always 1:1
            out += static_cast<char>(cp);
        } else {
            auto found = table.find(cp);
            if (found != table.end()) {
                out += static_cast<char>(found->second);
            } else {
                // Unmappable: keep original UTF-8 bytes
                out.append(before, it);
            }
        }
    }
    return out;
}

// ── Public API ─────────────────────────────────────────────────────────────────

std::string charsetConvert(const std::string& utf8text, OutputCharset charset) {
    switch (charset) {
        case OutputCharset::Unicode: return utf8text;
        case OutputCharset::TCVN3:   return convertWithTable(utf8text, tcvn3Table());
        case OutputCharset::CP1258:  return convertWithTable(utf8text, cp1258Table());
        case OutputCharset::VISCII:  return convertWithTable(utf8text, visciiTable());
    }
    return utf8text;
}

}  // namespace farolkey::core::charset
