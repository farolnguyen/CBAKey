#pragma once

#include <string>

namespace farolkey::core::charset {

enum class OutputCharset {
    Unicode,  // UTF-8, no-op (default)
    TCVN3,    // TCVN3-1993 (ABC font encoding)
    CP1258,   // Windows-1258 (Vietnamese code page)
    VISCII,   // VISCII-1.1
};

// Convert a UTF-8 string to the target charset.
// ASCII bytes (0x00-0x7F) are passed through unchanged.
// Vietnamese Unicode codepoints are mapped via charset-specific table.
// Unmappable non-ASCII codepoints are passed through as UTF-8.
// When charset == Unicode: returns utf8text unchanged (no-op).
std::string charsetConvert(const std::string& utf8text, OutputCharset charset);

}  // namespace farolkey::core::charset
