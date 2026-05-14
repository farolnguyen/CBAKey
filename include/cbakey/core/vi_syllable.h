#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace cbakey::core::vi_syllable {

struct SyllableSpan {
    std::size_t begin = 0;
    std::size_t onset_end = 0;
    std::size_t medial_end = 0;
    std::size_t nucleus_end = 0;
    std::size_t end = 0;
};

struct StableComposeSplit {
    std::size_t committed_prefix_end = 0;
    SyllableSpan active_suffix;
};

// Returns the longest valid syllable suffix that ends at buffer.back().
std::optional<SyllableSpan> findLastSyllable(const std::u32string& buffer);

// Returns a conservative split when the whole buffer can be segmented into
// multiple valid syllables and the final editable suffix already looks like a
// new syllable with an onset. The prefix before `committed_prefix_end` is then
// stable enough to auto-commit.
std::optional<StableComposeSplit> findStableComposeSplit(const std::u32string& buffer);

// Like `segmentWholeBuffer` but prefers the segmentation with the **most**
// syllables when ambiguous (used for remove-diacritics so only the last
// syllable is stripped, e.g. "chủân" -> "chủan" instead of "chuan").
std::optional<std::vector<SyllableSpan>> segmentWholeBufferPreferMaxSyllables(const std::u32string& buffer);

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

// Normalizes Vietnamese text into an NFC-like precomposed form for the rows
// the engine handles (`ă â ê ô ơ ư` plus tone marks).
bool normalizeVietnameseNfc(std::u32string& buffer);

// Removes Vietnamese diacritics in-place for Telex `z` handling, including
// tone marks and transformed vowel rows (`ă â ê ô ơ ư đ` -> `a a e o o u d`).
bool removeTelexDiacritics(std::u32string& buffer);

// Removes Vietnamese diacritics in-place for VNI `0` handling.
bool removeVniDiacritics(std::u32string& buffer);

// Applies VNI transform on the last syllable when the parser is confident.
// Returns false so callers can fall back to legacy heuristics.
bool applyVniTransform(std::u32string& buffer, char key);

}  // namespace cbakey::core::vi_syllable
