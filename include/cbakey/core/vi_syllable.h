#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace cbakey::core::vi_syllable {

struct SyllableSpan {
    std::size_t begin = 0;
    std::size_t onset_end = 0;
    std::size_t medial_end = 0;
    std::size_t nucleus_end = 0;
    std::size_t end = 0;
};

// Returns the longest valid syllable suffix that ends at buffer.back().
std::optional<SyllableSpan> findLastSyllable(const std::u32string& buffer);

// Returns the absolute buffer index of the tone-bearing vowel when the parser is
// confident. Returns nullopt to let callers fall back to legacy heuristics.
std::optional<std::size_t> selectToneVowelIndex(const std::u32string& buffer);

// Applies Telex transform on the last syllable when the parser is confident.
// Returns false so callers can fall back to legacy heuristics.
bool applyTelexTransform(std::u32string& buffer, char key);

// Normalizes Telex-in-progress syllables after appending literal characters,
// e.g. upgrades pending `uơ` into `ươ...` once the syllable gains a coda or an
// extra vowel.
bool normalizeTelexBuffer(std::u32string& buffer);

// Applies VNI transform on the last syllable when the parser is confident.
// Returns false so callers can fall back to legacy heuristics.
bool applyVniTransform(std::u32string& buffer, char key);

}  // namespace cbakey::core::vi_syllable
