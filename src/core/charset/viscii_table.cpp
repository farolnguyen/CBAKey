#include <cstdint>
#include <unordered_map>

namespace farolkey::core::charset {

// VISCII (RFC 1456): Unicode NFC codepoint -> single VISCII byte.
// Generated from iconv VISCII (byte <-> codepoint scan over 0x00-0xFF).
// ASCII (0x00-0x7F) passes through unchanged and is not in this table,
// EXCEPT that VISCII reassigns 6 control-character byte positions
// (0x02, 0x05, 0x06, 0x14, 0x19, 0x1E) to Vietnamese letters (Ẳ, Ẵ, Ẫ, Ỷ,
// Ỹ, Ỵ). All 134 Vietnamese letters have a unique single byte — no
// decomposition is needed (unlike CP1258).
const std::unordered_map<char32_t, uint8_t>& visciiTable() {
    static const std::unordered_map<char32_t, uint8_t> kTable{
        // ── Vietnamese-specific base characters (no tone) ──────────────────────
        {0x0102, 0xC5},  // Ă
        {0x0103, 0xE5},  // ă
        {0x00C2, 0xC2},  // Â
        {0x00E2, 0xE2},  // â
        {0x00CA, 0xCA},  // Ê
        {0x00EA, 0xEA},  // ê
        {0x00D4, 0xD4},  // Ô
        {0x00F4, 0xF4},  // ô
        {0x01A0, 0xB4},  // Ơ
        {0x01A1, 0xBD},  // ơ
        {0x01AF, 0xBF},  // Ư
        {0x01B0, 0xDF},  // ư
        {0x0110, 0xD0},  // Đ
        {0x0111, 0xF0},  // đ
        // ── Latin a/e/i/o/u/y + huyen (grave) / sac (acute) ─────────────────────
        {0x00C0, 0xC0},  // À
        {0x00E0, 0xE0},  // à
        {0x00C1, 0xC1},  // Á
        {0x00E1, 0xE1},  // á
        {0x00C8, 0xC8},  // È
        {0x00E8, 0xE8},  // è
        {0x00C9, 0xC9},  // É
        {0x00E9, 0xE9},  // é
        {0x00CC, 0xCC},  // Ì
        {0x00EC, 0xEC},  // ì
        {0x00CD, 0xCD},  // Í
        {0x00ED, 0xED},  // í
        {0x00D2, 0xD2},  // Ò
        {0x00F2, 0xF2},  // ò
        {0x00D3, 0xD3},  // Ó
        {0x00F3, 0xF3},  // ó
        {0x00D9, 0xD9},  // Ù
        {0x00F9, 0xF9},  // ù
        {0x00DA, 0xDA},  // Ú
        {0x00FA, 0xFA},  // ú
        {0x00DD, 0xDD},  // Ý
        {0x00FD, 0xFD},  // ý
        // ── A plain + hoi/nga/nang (Ả Ã Ạ) ──────────────────────────────────────
        {0x1EA2, 0xC4},  // Ả
        {0x1EA3, 0xE4},  // ả
        {0x00C3, 0xC3},  // Ã
        {0x00E3, 0xE3},  // ã
        {0x1EA0, 0x80},  // Ạ
        {0x1EA1, 0xD5},  // ạ
        // ── E plain + hoi/nga/nang (Ẻ Ẽ Ẹ) ──────────────────────────────────────
        {0x1EBA, 0xCB},  // Ẻ
        {0x1EBB, 0xEB},  // ẻ
        {0x1EBC, 0x88},  // Ẽ
        {0x1EBD, 0xA8},  // ẽ
        {0x1EB8, 0x89},  // Ẹ
        {0x1EB9, 0xA9},  // ẹ
        // ── I plain + nga/hoi/nang (Ĩ Ỉ Ị) ──────────────────────────────────────
        {0x0128, 0xCE},  // Ĩ
        {0x0129, 0xEE},  // ĩ
        {0x1EC8, 0x9B},  // Ỉ
        {0x1EC9, 0xEF},  // ỉ
        {0x1ECA, 0x98},  // Ị
        {0x1ECB, 0xB8},  // ị
        // ── O plain + nga/hoi/nang (Õ Ỏ Ọ) ──────────────────────────────────────
        {0x00D5, 0xA0},  // Õ
        {0x00F5, 0xF5},  // õ
        {0x1ECE, 0x99},  // Ỏ
        {0x1ECF, 0xF6},  // ỏ
        {0x1ECC, 0x9A},  // Ọ
        {0x1ECD, 0xF7},  // ọ
        // ── U plain + nga/hoi/nang (Ũ Ủ Ụ) ──────────────────────────────────────
        {0x0168, 0x9D},  // Ũ
        {0x0169, 0xFB},  // ũ
        {0x1EE6, 0x9C},  // Ủ
        {0x1EE7, 0xFC},  // ủ
        {0x1EE4, 0x9E},  // Ụ
        {0x1EE5, 0xF8},  // ụ
        // ── Y plain + huyen/hoi/nga/nang (Ỳ Ỷ Ỹ Ỵ) ───────────────────────────────
        {0x1EF2, 0x9F},  // Ỳ
        {0x1EF3, 0xCF},  // ỳ
        {0x1EF6, 0x14},  // Ỷ
        {0x1EF7, 0xD6},  // ỷ
        {0x1EF8, 0x19},  // Ỹ
        {0x1EF9, 0xDB},  // ỹ
        {0x1EF4, 0x1E},  // Ỵ
        {0x1EF5, 0xDC},  // ỵ
        // ── Â family (â + 5 thanh dieu) ─────────────────────────────────────────
        {0x1EA4, 0x84},  // Ấ
        {0x1EA5, 0xA4},  // ấ
        {0x1EA6, 0x85},  // Ầ
        {0x1EA7, 0xA5},  // ầ
        {0x1EA8, 0x86},  // Ẩ
        {0x1EA9, 0xA6},  // ẩ
        {0x1EAA, 0x06},  // Ẫ
        {0x1EAB, 0xE7},  // ẫ
        {0x1EAC, 0x87},  // Ậ
        {0x1EAD, 0xA7},  // ậ
        // ── Ă family (ă + 5 thanh dieu) ─────────────────────────────────────────
        {0x1EAE, 0x81},  // Ắ
        {0x1EAF, 0xA1},  // ắ
        {0x1EB0, 0x82},  // Ằ
        {0x1EB1, 0xA2},  // ằ
        {0x1EB2, 0x02},  // Ẳ
        {0x1EB3, 0xC6},  // ẳ
        {0x1EB4, 0x05},  // Ẵ
        {0x1EB5, 0xC7},  // ẵ
        {0x1EB6, 0x83},  // Ặ
        {0x1EB7, 0xA3},  // ặ
        // ── Ê family (ê + 5 thanh dieu) ─────────────────────────────────────────
        {0x1EBE, 0x8A},  // Ế
        {0x1EBF, 0xAA},  // ế
        {0x1EC0, 0x8B},  // Ề
        {0x1EC1, 0xAB},  // ề
        {0x1EC2, 0x8C},  // Ể
        {0x1EC3, 0xAC},  // ể
        {0x1EC4, 0x8D},  // Ễ
        {0x1EC5, 0xAD},  // ễ
        {0x1EC6, 0x8E},  // Ệ
        {0x1EC7, 0xAE},  // ệ
        // ── Ô family (ô + 5 thanh dieu) ──────────────────────────────────────────
        {0x1ED0, 0x8F},  // Ố
        {0x1ED1, 0xAF},  // ố
        {0x1ED2, 0x90},  // Ồ
        {0x1ED3, 0xB0},  // ồ
        {0x1ED4, 0x91},  // Ổ
        {0x1ED5, 0xB1},  // ổ
        {0x1ED6, 0x92},  // Ỗ
        {0x1ED7, 0xB2},  // ỗ
        {0x1ED8, 0x93},  // Ộ
        {0x1ED9, 0xB5},  // ộ
        // ── Ơ family (ơ + 5 thanh dieu) ──────────────────────────────────────────
        {0x1EDA, 0x95},  // Ớ
        {0x1EDB, 0xBE},  // ớ
        {0x1EDC, 0x96},  // Ờ
        {0x1EDD, 0xB6},  // ờ
        {0x1EDE, 0x97},  // Ở
        {0x1EDF, 0xB7},  // ở
        {0x1EE0, 0xB3},  // Ỡ
        {0x1EE1, 0xDE},  // ỡ
        {0x1EE2, 0x94},  // Ợ
        {0x1EE3, 0xFE},  // ợ
        // ── Ư family (ư + 5 thanh dieu) ──────────────────────────────────────────
        {0x1EE8, 0xBA},  // Ứ
        {0x1EE9, 0xD1},  // ứ
        {0x1EEA, 0xBB},  // Ừ
        {0x1EEB, 0xD7},  // ừ
        {0x1EEC, 0xBC},  // Ử
        {0x1EED, 0xD8},  // ử
        {0x1EEE, 0xFF},  // Ữ
        {0x1EEF, 0xE6},  // ữ
        {0x1EF0, 0xB9},  // Ự
        {0x1EF1, 0xF1},  // ự
    };
    return kTable;
}

}  // namespace farolkey::core::charset
