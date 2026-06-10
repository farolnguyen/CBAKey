#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "farolkey/core/charset/output_charset.h"

using farolkey::core::charset::OutputCharset;
using farolkey::core::charset::charsetConvert;

// ── Helpers ────────────────────────────────────────────────────────────────────

// Build expected bytes from a list of uint8_t values.
static std::string bytes(std::initializer_list<uint8_t> vals) {
    std::string s;
    for (uint8_t v : vals) s += static_cast<char>(v);
    return s;
}

// ── Unicode no-op ──────────────────────────────────────────────────────────────

TEST(charset_unicode, passthrough) {
    std::string input = "tiếng Việt 123";
    EXPECT_EQ(charsetConvert(input, OutputCharset::Unicode), input);
}

TEST(charset_unicode, empty) {
    EXPECT_EQ(charsetConvert("", OutputCharset::Unicode), "");
}

// ── TCVN3 ──────────────────────────────────────────────────────────────────────

TEST(charset_tcvn3, ascii_passthrough) {
    std::string ascii = "Hello, world! 0123";
    EXPECT_EQ(charsetConvert(ascii, OutputCharset::TCVN3), ascii);
}

TEST(charset_tcvn3, base_chars) {
    // ă Ă â Â ê Ê ô Ô ơ Ơ ư Ư đ Đ
    EXPECT_EQ(charsetConvert("ă", OutputCharset::TCVN3), bytes({0xA8}));
    EXPECT_EQ(charsetConvert("Ă", OutputCharset::TCVN3), bytes({0xA1}));
    EXPECT_EQ(charsetConvert("â", OutputCharset::TCVN3), bytes({0xA9}));
    EXPECT_EQ(charsetConvert("Â", OutputCharset::TCVN3), bytes({0xA2}));
    EXPECT_EQ(charsetConvert("ê", OutputCharset::TCVN3), bytes({0xAA}));
    EXPECT_EQ(charsetConvert("ô", OutputCharset::TCVN3), bytes({0xAB}));
    EXPECT_EQ(charsetConvert("ơ", OutputCharset::TCVN3), bytes({0xAC}));
    EXPECT_EQ(charsetConvert("ư", OutputCharset::TCVN3), bytes({0xAD}));
    EXPECT_EQ(charsetConvert("đ", OutputCharset::TCVN3), bytes({0xAE}));
    EXPECT_EQ(charsetConvert("Đ", OutputCharset::TCVN3), bytes({0xA7}));
}

TEST(charset_tcvn3, tones_a) {
    EXPECT_EQ(charsetConvert("à", OutputCharset::TCVN3), bytes({0xB5}));  // à
    EXPECT_EQ(charsetConvert("á", OutputCharset::TCVN3), bytes({0xB8}));  // á
    EXPECT_EQ(charsetConvert("ả", OutputCharset::TCVN3), bytes({0xB6}));  // ả
    EXPECT_EQ(charsetConvert("ã", OutputCharset::TCVN3), bytes({0xB7}));  // ã
    EXPECT_EQ(charsetConvert("ạ", OutputCharset::TCVN3), bytes({0xB9}));  // ạ
}

TEST(charset_tcvn3, circumflex_family) {
    // â family
    EXPECT_EQ(charsetConvert("ầ", OutputCharset::TCVN3), bytes({0xC7}));  // ầ
    EXPECT_EQ(charsetConvert("ấ", OutputCharset::TCVN3), bytes({0xCA}));  // ấ
    EXPECT_EQ(charsetConvert("ậ", OutputCharset::TCVN3), bytes({0xCB}));  // ậ
    // ô family
    EXPECT_EQ(charsetConvert("ồ", OutputCharset::TCVN3), bytes({0xE5}));  // ồ
    EXPECT_EQ(charsetConvert("ố", OutputCharset::TCVN3), bytes({0xE8}));  // ố
    EXPECT_EQ(charsetConvert("ộ", OutputCharset::TCVN3), bytes({0xE9}));  // ộ
    // ê family
    EXPECT_EQ(charsetConvert("ề", OutputCharset::TCVN3), bytes({0xD2}));  // ề
    EXPECT_EQ(charsetConvert("ế", OutputCharset::TCVN3), bytes({0xD5}));  // ế
    EXPECT_EQ(charsetConvert("ệ", OutputCharset::TCVN3), bytes({0xD6}));  // ệ
}

TEST(charset_tcvn3, horn_family) {
    // ơ family
    EXPECT_EQ(charsetConvert("ờ", OutputCharset::TCVN3), bytes({0xEA}));  // ờ
    EXPECT_EQ(charsetConvert("ớ", OutputCharset::TCVN3), bytes({0xED}));  // ớ
    EXPECT_EQ(charsetConvert("ợ", OutputCharset::TCVN3), bytes({0xEE}));  // ợ
    // ư family
    EXPECT_EQ(charsetConvert("ừ", OutputCharset::TCVN3), bytes({0xF5}));  // ừ
    EXPECT_EQ(charsetConvert("ứ", OutputCharset::TCVN3), bytes({0xF8}));  // ứ
    EXPECT_EQ(charsetConvert("ự", OutputCharset::TCVN3), bytes({0xF9}));  // ự
}

TEST(charset_tcvn3, words) {
    // "tiếng" = t + i + ế + n + g
    EXPECT_EQ(charsetConvert("tiếng", OutputCharset::TCVN3),
              bytes({'t', 'i', 0xD5, 'n', 'g'}));

    // "việt" = v + i + ệ + t
    EXPECT_EQ(charsetConvert("việt", OutputCharset::TCVN3),
              bytes({'v', 'i', 0xD6, 't'}));

    // "được" = đ + ư + ợ + c
    EXPECT_EQ(charsetConvert("được", OutputCharset::TCVN3),
              bytes({0xAE, 0xAD, 0xEE, 'c'}));

    // "hoặc" = h + o + ặ + c
    EXPECT_EQ(charsetConvert("hoặc", OutputCharset::TCVN3),
              bytes({'h', 'o', 0xC6, 'c'}));
}

TEST(charset_tcvn3, unmappable_passthrough) {
    // Characters missing from TCVN3 (e.g. Ứ uppercase) fall back to UTF-8
    std::string u_horn_acute = "\xE1\xBB\xA8";  // Ứ U+1EE8 in UTF-8
    std::string result = charsetConvert(u_horn_acute, OutputCharset::TCVN3);
    EXPECT_EQ(result, u_horn_acute);  // kept as UTF-8
}

TEST(charset_tcvn3, mixed_ascii_and_vietnamese) {
    // "Xin chào!" → X,i,n,' ',c,h,à,o,'!'
    EXPECT_EQ(charsetConvert("Xin chào!", OutputCharset::TCVN3),
              bytes({'X', 'i', 'n', ' ', 'c', 'h', 0xB5, 'o', '!'}));
}

// ── CP1258 ───────────────────────────────────────────────────────────────────

TEST(charset_cp1258, ascii_passthrough) {
    std::string ascii = "Hello, world! 0123";
    EXPECT_EQ(charsetConvert(ascii, OutputCharset::CP1258), ascii);
}

TEST(charset_cp1258, base_chars_direct) {
    // ă Â â Ê ê Ô ô Ơ ơ Ư ư đ Đ — single-byte direct mappings
    EXPECT_EQ(charsetConvert("ă", OutputCharset::CP1258), bytes({0xE3}));
    EXPECT_EQ(charsetConvert("Â", OutputCharset::CP1258), bytes({0xC2}));
    EXPECT_EQ(charsetConvert("â", OutputCharset::CP1258), bytes({0xE2}));
    EXPECT_EQ(charsetConvert("Ê", OutputCharset::CP1258), bytes({0xCA}));
    EXPECT_EQ(charsetConvert("ê", OutputCharset::CP1258), bytes({0xEA}));
    EXPECT_EQ(charsetConvert("ô", OutputCharset::CP1258), bytes({0xF4}));
    EXPECT_EQ(charsetConvert("Ơ", OutputCharset::CP1258), bytes({0xD5}));
    EXPECT_EQ(charsetConvert("ơ", OutputCharset::CP1258), bytes({0xF5}));
    EXPECT_EQ(charsetConvert("Ư", OutputCharset::CP1258), bytes({0xDD}));
    EXPECT_EQ(charsetConvert("ư", OutputCharset::CP1258), bytes({0xFD}));
    EXPECT_EQ(charsetConvert("đ", OutputCharset::CP1258), bytes({0xF0}));
    EXPECT_EQ(charsetConvert("Đ", OutputCharset::CP1258), bytes({0xD0}));
}

TEST(charset_cp1258, tones_direct_single_byte) {
    // à á è é í ó ù ú have direct single-byte CP1258 forms
    EXPECT_EQ(charsetConvert("à", OutputCharset::CP1258), bytes({0xE0}));
    EXPECT_EQ(charsetConvert("á", OutputCharset::CP1258), bytes({0xE1}));
    EXPECT_EQ(charsetConvert("é", OutputCharset::CP1258), bytes({0xE9}));
    EXPECT_EQ(charsetConvert("ú", OutputCharset::CP1258), bytes({0xFA}));
}

TEST(charset_cp1258, tones_decomposed_two_bytes) {
    // ã ì ò õ ý have NO single CP1258 byte: base letter + combining-mark byte
    EXPECT_EQ(charsetConvert("ã", OutputCharset::CP1258), bytes({0x61, 0xDE}));  // a + tilde
    EXPECT_EQ(charsetConvert("ì", OutputCharset::CP1258), bytes({0x69, 0xCC}));  // i + grave
    EXPECT_EQ(charsetConvert("ò", OutputCharset::CP1258), bytes({0x6F, 0xCC}));  // o + grave
    EXPECT_EQ(charsetConvert("õ", OutputCharset::CP1258), bytes({0x6F, 0xDE}));  // o + tilde
    EXPECT_EQ(charsetConvert("ý", OutputCharset::CP1258), bytes({0x79, 0xEC}));  // y + acute
}

TEST(charset_cp1258, circumflex_and_horn_family_decomposed) {
    // â/ê/ô/ơ/ư + tone -> precomposed-base byte + combining-mark byte
    EXPECT_EQ(charsetConvert("ấ", OutputCharset::CP1258), bytes({0xE2, 0xEC}));  // â + acute
    EXPECT_EQ(charsetConvert("ầ", OutputCharset::CP1258), bytes({0xE2, 0xCC}));  // â + grave
    EXPECT_EQ(charsetConvert("ậ", OutputCharset::CP1258), bytes({0xE2, 0xF2}));  // â + dot below
    EXPECT_EQ(charsetConvert("ờ", OutputCharset::CP1258), bytes({0xF5, 0xCC}));  // ơ + grave
    EXPECT_EQ(charsetConvert("ữ", OutputCharset::CP1258), bytes({0xFD, 0xDE}));  // ư + tilde
    EXPECT_EQ(charsetConvert("Ứ", OutputCharset::CP1258), bytes({0xDD, 0xEC}));  // Ư + acute
}

TEST(charset_cp1258, words) {
    // "tiếng" = t + i + ế(ê+acute) + n + g
    EXPECT_EQ(charsetConvert("tiếng", OutputCharset::CP1258),
              bytes({'t', 'i', 0xEA, 0xEC, 'n', 'g'}));

    // "việt" = v + i + ệ(ê+dot below) + t
    EXPECT_EQ(charsetConvert("việt", OutputCharset::CP1258),
              bytes({'v', 'i', 0xEA, 0xF2, 't'}));

    // "được" = đ + ư + ợ(ơ+dot below) + c
    EXPECT_EQ(charsetConvert("được", OutputCharset::CP1258),
              bytes({0xF0, 0xFD, 0xF5, 0xF2, 'c'}));

    // "hoặc" = h + o + ặ(ă+dot below) + c
    EXPECT_EQ(charsetConvert("hoặc", OutputCharset::CP1258),
              bytes({'h', 'o', 0xE3, 0xF2, 'c'}));
}

TEST(charset_cp1258, unmappable_passthrough) {
    // Non-Vietnamese, non-Latin-1 codepoints fall back to UTF-8
    std::string han_char = "\xE4\xB8\xAD";  // 中 U+4E2D in UTF-8
    EXPECT_EQ(charsetConvert(han_char, OutputCharset::CP1258), han_char);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
