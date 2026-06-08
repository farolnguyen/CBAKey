#include <cstdint>
#include <unordered_map>

namespace farolkey::core::charset {

// TCVN3-1993 (TCVN5712-1:1993 / ABC font encoding).
// Unicode NFC codepoint → TCVN3 byte.
// Generated from iconv TCVN5712-1:1993 tables; 122 entries.
// ASCII (0x00–0x7F) passes through unchanged and is not in this table.
// Characters not present here are emitted as UTF-8 (e.g. uppercase Ú, Ứ, Ừ, …
// which are absent from the TCVN3 standard).
const std::unordered_map<char32_t, uint8_t>& tcvn3Table() {
    static const std::unordered_map<char32_t, uint8_t> kTable{
        // ── Latin base vowels (plain, no Vietnamese diacritic) ─────────────────
        {0x00C0, 0x80},  // À
        {0x00C1, 0x83},  // Á
        {0x00C2, 0xA2},  // Â
        {0x00C3, 0x82},  // Ã
        {0x00C8, 0x87},  // È
        {0x00C9, 0x8A},  // É
        {0x00CA, 0xA3},  // Ê
        {0x00CC, 0x8D},  // Ì
        {0x00CD, 0x90},  // Í
        {0x00D2, 0x92},  // Ò
        {0x00D3, 0x95},  // Ó
        {0x00D4, 0xA4},  // Ô
        {0x00D5, 0x94},  // Õ
        {0x00D9, 0x9D},  // Ù
        {0x00E0, 0xB5},  // à
        {0x00E1, 0xB8},  // á
        {0x00E2, 0xA9},  // â
        {0x00E3, 0xB7},  // ã
        {0x00E8, 0xCC},  // è
        {0x00E9, 0xD0},  // é
        {0x00EA, 0xAA},  // ê
        {0x00EC, 0xD7},  // ì
        {0x00ED, 0xDD},  // í
        {0x00F2, 0xDF},  // ò
        {0x00F3, 0xE3},  // ó
        {0x00F4, 0xAB},  // ô
        {0x00F5, 0xE2},  // õ
        {0x00F9, 0xEF},  // ù
        {0x00FA, 0xF3},  // ú
        {0x00FD, 0xFD},  // ý
        // ── Vietnamese-specific base characters ────────────────────────────────
        {0x0102, 0xA1},  // Ă
        {0x0103, 0xA8},  // ă
        {0x0110, 0xA7},  // Đ
        {0x0111, 0xAE},  // đ
        {0x0128, 0x8F},  // Ĩ
        {0x0129, 0xDC},  // ĩ
        {0x0168, 0x9F},  // Ũ
        {0x0169, 0xF2},  // ũ
        {0x01A0, 0xA5},  // Ơ
        {0x01A1, 0xAC},  // ơ
        {0x01AF, 0xA6},  // Ư
        {0x01B0, 0xAD},  // ư
        // ── A with circumflex + tone (Â family) ───────────────────────────────
        {0x1EA4, 0xC4},  // Ấ
        {0x1EA5, 0xCA},  // ấ
        {0x1EA6, 0xC1},  // Ầ
        {0x1EA7, 0xC7},  // ầ
        {0x1EA8, 0xC2},  // Ẩ
        {0x1EA9, 0xC8},  // ẩ
        {0x1EAA, 0xC3},  // Ẫ
        {0x1EAB, 0xC9},  // ẫ
        {0x1EAC, 0x86},  // Ậ
        {0x1EAD, 0xCB},  // ậ
        // ── A with breve + tone (Ă family) ────────────────────────────────────
        {0x1EAE, 0xC0},  // Ắ
        {0x1EAF, 0xBE},  // ắ
        {0x1EB0, 0xAF},  // Ằ
        {0x1EB1, 0xBB},  // ằ
        {0x1EB2, 0xBA},  // Ẳ
        {0x1EB3, 0xBC},  // ẳ
        {0x1EB4, 0xBF},  // Ẵ
        {0x1EB5, 0xBD},  // ẵ
        {0x1EB6, 0x85},  // Ặ
        {0x1EB7, 0xC6},  // ặ
        // ── A plain + tone ─────────────────────────────────────────────────────
        {0x1EA0, 0x84},  // Ạ
        {0x1EA1, 0xB9},  // ạ
        {0x1EA2, 0x81},  // Ả
        {0x1EA3, 0xB6},  // ả
        // ── E plain + tone ─────────────────────────────────────────────────────
        {0x1EB8, 0x8B},  // Ẹ
        {0x1EB9, 0xD1},  // ẹ
        {0x1EBA, 0x88},  // Ẻ
        {0x1EBB, 0xCE},  // ẻ
        {0x1EBC, 0x89},  // Ẽ
        {0x1EBD, 0xCF},  // ẽ
        // ── E with circumflex + tone (Ê family) ───────────────────────────────
        {0x1EBE, 0xDA},  // Ế
        {0x1EBF, 0xD5},  // ế
        {0x1EC0, 0xC5},  // Ề
        {0x1EC1, 0xD2},  // ề
        {0x1EC2, 0xCD},  // Ể
        {0x1EC3, 0xD3},  // ể
        {0x1EC4, 0xD9},  // Ễ
        {0x1EC5, 0xD4},  // ễ
        {0x1EC6, 0x8C},  // Ệ
        {0x1EC7, 0xD6},  // ệ
        // ── I plain + tone ─────────────────────────────────────────────────────
        {0x1EC8, 0x8E},  // Ỉ
        {0x1EC9, 0xD8},  // ỉ
        {0x1ECA, 0x91},  // Ị
        {0x1ECB, 0xDE},  // ị
        // ── O plain + tone ─────────────────────────────────────────────────────
        {0x1ECC, 0x96},  // Ọ
        {0x1ECD, 0xE4},  // ọ
        {0x1ECE, 0x93},  // Ỏ
        {0x1ECF, 0xE1},  // ỏ
        // ── O with circumflex + tone (Ô family) ───────────────────────────────
        {0x1ED0, 0xFF},  // Ố
        {0x1ED1, 0xE8},  // ố
        {0x1ED2, 0xDB},  // Ồ
        {0x1ED3, 0xE5},  // ồ
        {0x1ED4, 0xE0},  // Ổ
        {0x1ED5, 0xE6},  // ổ
        {0x1ED6, 0xF0},  // Ỗ
        {0x1ED7, 0xE7},  // ỗ
        {0x1ED8, 0x97},  // Ộ
        {0x1ED9, 0xE9},  // ộ
        // ── O with horn + tone (Ơ family) ─────────────────────────────────────
        {0x1EDA, 0x9B},  // Ớ
        {0x1EDB, 0xED},  // ớ
        {0x1EDC, 0x98},  // Ờ
        {0x1EDD, 0xEA},  // ờ
        {0x1EDE, 0x99},  // Ở
        {0x1EDF, 0xEB},  // ở
        {0x1EE0, 0x9A},  // Ỡ
        {0x1EE1, 0xEC},  // ỡ
        {0x1EE2, 0x9C},  // Ợ
        {0x1EE3, 0xEE},  // ợ
        // ── U plain + tone ─────────────────────────────────────────────────────
        {0x1EE5, 0xF4},  // ụ
        {0x1EE6, 0x9E},  // Ủ
        {0x1EE7, 0xF1},  // ủ
        // ── U with horn + tone (Ư family) ─────────────────────────────────────
        {0x1EE9, 0xF8},  // ứ
        {0x1EEB, 0xF5},  // ừ
        {0x1EED, 0xF6},  // ử
        {0x1EEF, 0xF7},  // ữ
        {0x1EF1, 0xF9},  // ự
        // ── Y + tone ───────────────────────────────────────────────────────────
        {0x1EF3, 0xFA},  // ỳ
        {0x1EF5, 0xFE},  // ỵ
        {0x1EF7, 0xFB},  // ỷ
        {0x1EF9, 0xFC},  // ỹ
    };
    return kTable;
}

}  // namespace farolkey::core::charset
