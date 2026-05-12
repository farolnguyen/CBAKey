#include "cbakey/core/vi_syllable.h"

#include <array>
#include <optional>
#include <string_view>
#include <vector>

namespace cbakey::core::vi_syllable {
namespace {

constexpr std::size_t kToneCount = 6;

const std::vector<std::array<char32_t, kToneCount>>& toneSets() {
    static const std::vector<std::array<char32_t, kToneCount>> sets = {
        {U'a', U'á', U'à', U'ả', U'ã', U'ạ'},
        {U'ă', U'ắ', U'ằ', U'ẳ', U'ẵ', U'ặ'},
        {U'â', U'ấ', U'ầ', U'ẩ', U'ẫ', U'ậ'},
        {U'e', U'é', U'è', U'ẻ', U'ẽ', U'ẹ'},
        {U'ê', U'ế', U'ề', U'ể', U'ễ', U'ệ'},
        {U'i', U'í', U'ì', U'ỉ', U'ĩ', U'ị'},
        {U'o', U'ó', U'ò', U'ỏ', U'õ', U'ọ'},
        {U'ô', U'ố', U'ồ', U'ổ', U'ỗ', U'ộ'},
        {U'ơ', U'ớ', U'ờ', U'ở', U'ỡ', U'ợ'},
        {U'u', U'ú', U'ù', U'ủ', U'ũ', U'ụ'},
        {U'ư', U'ứ', U'ừ', U'ử', U'ữ', U'ự'},
        {U'y', U'ý', U'ỳ', U'ỷ', U'ỹ', U'ỵ'}};
    return sets;
}

bool locateInToneSets(char32_t ch, std::size_t* outSetIdx = nullptr, std::size_t* outToneIdx = nullptr) {
    const auto& sets = toneSets();
    for (std::size_t si = 0; si < sets.size(); ++si) {
        for (std::size_t ti = 0; ti < sets[si].size(); ++ti) {
            const char32_t member = sets[si][ti];
            if (member == ch) {
                if (outSetIdx != nullptr) {
                    *outSetIdx = si;
                }
                if (outToneIdx != nullptr) {
                    *outToneIdx = ti;
                }
                return true;
            }
        }
    }
    return false;
}

char32_t baseChar(char32_t ch) {
    std::size_t si = 0;
    if (!locateInToneSets(ch, &si)) {
        return ch;
    }
    return toneSets()[si][0];
}

bool isVowel(char32_t ch) {
    return locateInToneSets(ch);
}

bool isVowelBase(char32_t ch) {
    switch (ch) {
        case U'a':
        case U'ă':
        case U'â':
        case U'e':
        case U'ê':
        case U'i':
        case U'o':
        case U'ô':
        case U'ơ':
        case U'u':
        case U'ư':
        case U'y':
            return true;
        default:
            return false;
    }
}

std::u32string baseSlice(const std::u32string& s, std::size_t begin, std::size_t end) {
    std::u32string out;
    out.reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i) {
        out.push_back(baseChar(s[i]));
    }
    return out;
}

struct TonePatternRule {
    std::u32string_view pattern;
    std::optional<std::size_t> offset_without_coda;
    std::optional<std::size_t> offset_with_coda;
};

bool startsWith(const std::u32string& s, std::u32string_view prefix) {
    if (s.size() < prefix.size()) {
        return false;
    }
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        if (s[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

bool endsWith(const std::u32string& s, std::u32string_view suffix) {
    if (s.size() < suffix.size()) {
        return false;
    }
    const std::size_t offset = s.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        if (s[offset + i] != suffix[i]) {
            return false;
        }
    }
    return true;
}

std::size_t longestOnsetLength(const std::u32string& base) {
    static constexpr std::u32string_view kOnsets[] = {
        U"ngh", U"ch", U"gh", U"kh", U"ng", U"nh", U"ph", U"th", U"tr",
        U"b",   U"c",  U"d",  U"đ",  U"g",  U"h",  U"k",  U"l",  U"m",
        U"n",   U"p",  U"q",  U"r",  U"s",  U"t",  U"v",  U"x"};
    for (std::u32string_view onset : kOnsets) {
        if (startsWith(base, onset)) {
            return onset.size();
        }
    }
    return 0;
}

std::size_t longestCodaLength(const std::u32string& base) {
    static constexpr std::u32string_view kCodas[] = {U"ch", U"nh", U"ng", U"c", U"m", U"n", U"p", U"t"};
    for (std::u32string_view coda : kCodas) {
        if (endsWith(base, coda)) {
            return coda.size();
        }
    }
    return 0;
}

std::optional<SyllableSpan> parseSingleSyllableRange(const std::u32string& buffer,
                                                     std::size_t begin,
                                                     std::size_t end) {
    if (begin >= end || end > buffer.size()) {
        return std::nullopt;
    }

    const std::u32string base = baseSlice(buffer, begin, end);
    const std::size_t onsetLen = longestOnsetLength(base);
    const std::size_t codaLen = longestCodaLength(base);
    if (onsetLen + codaLen >= base.size()) {
        return std::nullopt;
    }

    std::size_t medialLen = 0;
    const std::size_t coreBegin = onsetLen;
    const std::size_t coreEnd = base.size() - codaLen;
    if (coreBegin >= coreEnd) {
        return std::nullopt;
    }

    if (onsetLen == 1 && base[0] == U'q' && coreBegin + 1 < coreEnd && base[coreBegin] == U'u' &&
        isVowelBase(base[coreBegin + 1])) {
        medialLen = 1;
    } else if (onsetLen == 1 && base[0] == U'g' && coreBegin + 1 < coreEnd && base[coreBegin] == U'i' &&
               isVowelBase(base[coreBegin + 1])) {
        medialLen = 1;
    }

    const std::size_t nucleusBegin = coreBegin + medialLen;
    if (nucleusBegin >= coreEnd) {
        return std::nullopt;
    }
    for (std::size_t i = nucleusBegin; i < coreEnd; ++i) {
        if (!isVowel(buffer[begin + i])) {
            return std::nullopt;
        }
    }

    return SyllableSpan{
        .begin = begin,
        .onset_end = begin + onsetLen,
        .medial_end = begin + nucleusBegin,
        .nucleus_end = begin + coreEnd,
        .end = end,
    };
}

std::optional<std::vector<SyllableSpan>> segmentWholeBuffer(const std::u32string& buffer) {
    if (buffer.empty()) {
        return std::nullopt;
    }

    struct MemoEntry {
        bool computed = false;
        std::optional<std::vector<SyllableSpan>> value;
    };

    std::vector<MemoEntry> memo(buffer.size() + 1);
    const auto solve = [&](auto&& self, std::size_t begin) -> std::optional<std::vector<SyllableSpan>> {
        if (begin == buffer.size()) {
            return std::vector<SyllableSpan>{};
        }

        MemoEntry& entry = memo[begin];
        if (entry.computed) {
            return entry.value;
        }
        entry.computed = true;

        std::optional<std::vector<SyllableSpan>> best;
        for (std::size_t end = begin + 1; end <= buffer.size(); ++end) {
            const auto span = parseSingleSyllableRange(buffer, begin, end);
            if (!span) {
                continue;
            }
            const auto rest = self(self, end);
            if (!rest) {
                continue;
            }

            std::vector<SyllableSpan> candidate;
            candidate.reserve(rest->size() + 1);
            candidate.push_back(*span);
            candidate.insert(candidate.end(), rest->begin(), rest->end());

            if (!best || candidate.size() < best->size() ||
                (candidate.size() == best->size() && candidate.back().begin > best->back().begin)) {
                best = std::move(candidate);
            }
        }

        entry.value = best;
        return entry.value;
    };

    return solve(solve, 0);
}

std::optional<std::size_t> selectToneOffset(const std::u32string& tonePattern, bool hasCoda) {
    if (tonePattern.empty()) {
        return std::nullopt;
    }
    if (tonePattern.size() == 1) {
        return 0;
    }

    static constexpr TonePatternRule kToneRules[] = {
        {U"oa", 0, 1},   {U"oe", 0, 1},   {U"ue", 1, 1},   {U"uê", 1, 1},   {U"uy", 1, 1},
        {U"ao", 0, 0},   {U"au", 0, 0},   {U"âu", 0, 0},   {U"ay", 0, 0},
        {U"ây", 0, 0},   {U"ai", 0, 0},   {U"eo", 0, 0},   {U"eu", 0, 0},
        {U"êu", 0, 0},   {U"oi", 0, 0},   {U"ôi", 0, 0},
        {U"ơi", 0, 0},   {U"ui", 0, 0},   {U"ưi", 0, 0},   {U"ua", 0, 0},
        {U"ia", 0, 1},   {U"ie", 1, 1},   {U"iê", 1, 1},   {U"ya", 0, 1},
        {U"ye", 1, 1},
        {U"yê", 1, 1},   {U"uye", 2, 2},
        {U"uyê", 2, 2},  {U"ưa", 0, 0},   {U"uay", 1, 1},  {U"uây", 1, 1},
        {U"uơ", 1, 1},   {U"ươ", 1, 1},   {U"uo", 1, 1},   {U"ưo", 1, 1},   {U"uô", 1, 1},
        {U"uou", 1, 1},  {U"uơu", 1, 1},  {U"ưou", 1, 1},  {U"uoi", 1, 1},  {U"uơi", 1, 1},
        {U"oai", 1, 1},  {U"oay", 1, 1},  {U"uu", 0, 0},   {U"ưu", 0, 0},
        {U"ieu", 1, 1},
        {U"yeu", 1, 1},  {U"iêu", 1, 1},  {U"yêu", 1, 1},  {U"ươu", 1, 1},
        {U"uôi", 1, 1},
        {U"ươi", 1, 1},  {U"uya", 1, 1},
    };

    for (const TonePatternRule& rule : kToneRules) {
        if (tonePattern == rule.pattern) {
            return hasCoda ? rule.offset_with_coda : rule.offset_without_coda;
        }
    }

    return std::nullopt;
}

bool replaceWithSetPreserveTone(std::u32string& buffer, std::size_t pos, std::size_t setIdx) {
    std::size_t oldSetIdx = 0;
    std::size_t toneIdx = 0;
    if (!locateInToneSets(buffer[pos], &oldSetIdx, &toneIdx)) {
        return false;
    }
    (void)oldSetIdx;
    buffer[pos] = toneSets()[setIdx][toneIdx];
    return true;
}

std::optional<std::size_t> rightmostNucleusCharWithBase(const std::u32string& buffer,
                                                        const SyllableSpan& span,
                                                        std::u32string_view bases) {
    for (std::size_t i = span.nucleus_end; i > span.medial_end; --i) {
        const char32_t base = baseChar(buffer[i - 1]);
        for (char32_t want : bases) {
            if (base == want) {
                return i - 1;
            }
        }
    }
    return std::nullopt;
}

std::optional<std::size_t> transformedSetForCombiningMark(char32_t ch, char32_t mark) {
    switch (mark) {
        case U'\u0306':
            return baseChar(ch) == U'a' ? std::optional<std::size_t>(1) : std::nullopt;
        case U'\u0302':
            switch (baseChar(ch)) {
                case U'a':
                    return 2;
                case U'e':
                    return 4;
                case U'o':
                    return 7;
                default:
                    return std::nullopt;
            }
        case U'\u031b':
            switch (baseChar(ch)) {
                case U'o':
                    return 8;
                case U'u':
                    return 10;
                default:
                    return std::nullopt;
            }
        default:
            return std::nullopt;
    }
}

std::optional<std::size_t> toneIndexForCombiningMark(char32_t mark) {
    switch (mark) {
        case U'\u0301':
            return 1;
        case U'\u0300':
            return 2;
        case U'\u0309':
            return 3;
        case U'\u0303':
            return 4;
        case U'\u0323':
            return 5;
        default:
            return std::nullopt;
    }
}

bool applyCombiningMark(char32_t& ch, char32_t mark) {
    if (const auto setIdx = transformedSetForCombiningMark(ch, mark)) {
        std::size_t oldSetIdx = 0;
        std::size_t toneIdx = 0;
        if (!locateInToneSets(ch, &oldSetIdx, &toneIdx)) {
            return false;
        }
        (void)oldSetIdx;
        ch = toneSets()[*setIdx][toneIdx];
        return true;
    }

    if (const auto toneIdx = toneIndexForCombiningMark(mark)) {
        std::size_t setIdx = 0;
        std::size_t oldToneIdx = 0;
        if (!locateInToneSets(ch, &setIdx, &oldToneIdx)) {
            return false;
        }
        ch = toneSets()[setIdx][*toneIdx];
        return true;
    }

    return false;
}

}  // namespace

std::optional<SyllableSpan> findLastSyllable(const std::u32string& buffer) {
    std::optional<SyllableSpan> best;
    for (std::size_t begin = 0; begin < buffer.size(); ++begin) {
        const auto span = parseSingleSyllableRange(buffer, begin, buffer.size());
        if (!span) {
            continue;
        }
        if (!best || span->begin < best->begin) {
            best = span;
        }
    }
    return best;
}

std::optional<StableComposeSplit> findStableComposeSplit(const std::u32string& buffer) {
    const auto segmented = segmentWholeBuffer(buffer);
    if (!segmented || segmented->size() < 2) {
        return std::nullopt;
    }

    const SyllableSpan& last = segmented->back();
    if (last.begin == 0 || last.onset_end == last.begin) {
        return std::nullopt;
    }

    return StableComposeSplit{
        .committed_prefix_end = last.begin,
        .active_suffix = last,
    };
}

bool normalizeVietnameseNfc(std::u32string& buffer) {
    std::u32string normalized;
    normalized.reserve(buffer.size());
    for (const char32_t ch : buffer) {
        if (!normalized.empty() && applyCombiningMark(normalized.back(), ch)) {
            continue;
        }
        normalized.push_back(ch);
    }
    if (normalized == buffer) {
        return false;
    }
    buffer = std::move(normalized);
    return true;
}

std::optional<std::size_t> selectToneVowelIndex(const std::u32string& buffer) {
    const auto span = findLastSyllable(buffer);
    if (!span) {
        return std::nullopt;
    }

    std::vector<std::size_t> candidateIndices;
    std::u32string tonePattern;
    for (std::size_t i = span->medial_end; i < span->nucleus_end; ++i) {
        if (!isVowel(buffer[i])) {
            return std::nullopt;
        }
        candidateIndices.push_back(i);
        tonePattern.push_back(baseChar(buffer[i]));
    }
    if (candidateIndices.empty()) {
        return std::nullopt;
    }

    const bool hasCoda = span->nucleus_end < span->end;
    const auto offset = selectToneOffset(tonePattern, hasCoda);
    if (!offset || *offset >= candidateIndices.size()) {
        return std::nullopt;
    }
    return candidateIndices[*offset];
}

bool applyTelexTransform(std::u32string& buffer, char key) {
    if (buffer.empty()) {
        return false;
    }
    normalizeVietnameseNfc(buffer);
    if (key == 'd' && buffer.back() == U'd') {
        buffer.back() = U'đ';
        return true;
    }

    const auto span = findLastSyllable(buffer);
    if (!span) {
        return false;
    }

    const std::u32string tonePattern = baseSlice(buffer, span->medial_end, span->nucleus_end);
    if (key == 'w' && tonePattern == U"uo" && span->nucleus_end - span->medial_end == 2) {
        return replaceWithSetPreserveTone(buffer, span->medial_end + 1, 8);
    }

    std::optional<std::size_t> pos;
    std::size_t newSetIdx = 0;
    if (key == 'a') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"a");
        newSetIdx = 2;
    } else if (key == 'e') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"e");
        newSetIdx = 4;
    } else if (key == 'o') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"o");
        newSetIdx = 7;
    } else if (key == 'w') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"aou");
        if (!pos) {
            return false;
        }
        const char32_t base = baseChar(buffer[*pos]);
        if (base == U'a') {
            newSetIdx = 1;
        } else if (base == U'o') {
            newSetIdx = 8;
        } else if (base == U'u') {
            newSetIdx = 10;
        } else {
            return false;
        }
    } else {
        return false;
    }

    if (!pos) {
        return false;
    }
    return replaceWithSetPreserveTone(buffer, *pos, newSetIdx);
}

bool normalizeTelexBuffer(std::u32string& buffer) {
    normalizeVietnameseNfc(buffer);
    const auto span = findLastSyllable(buffer);
    if (!span) {
        return false;
    }

    const std::u32string tonePattern = baseSlice(buffer, span->medial_end, span->nucleus_end);
    const bool hasCoda = span->nucleus_end < span->end;
    if (!startsWith(tonePattern, U"uơ")) {
        return false;
    }
    if (!hasCoda && tonePattern.size() <= 2) {
        return false;
    }
    return replaceWithSetPreserveTone(buffer, span->medial_end, 10);
}

bool removeVietnameseDiacritics(std::u32string& buffer) {
    normalizeVietnameseNfc(buffer);
    bool changed = false;
    for (char32_t& ch : buffer) {
        if (ch == U'đ') {
            ch = U'd';
            changed = true;
            continue;
        }

        const char32_t base = baseChar(ch);
        char32_t normalized = ch;
        switch (base) {
            case U'ă':
            case U'â':
                normalized = U'a';
                break;
            case U'ê':
                normalized = U'e';
                break;
            case U'ô':
            case U'ơ':
                normalized = U'o';
                break;
            case U'ư':
                normalized = U'u';
                break;
            default:
                normalized = base;
                break;
        }
        if (normalized != ch) {
            ch = normalized;
            changed = true;
        }
    }
    return changed;
}

bool removeTelexDiacritics(std::u32string& buffer) {
    return removeVietnameseDiacritics(buffer);
}

bool removeVniDiacritics(std::u32string& buffer) {
    return removeVietnameseDiacritics(buffer);
}

bool applyVniTransform(std::u32string& buffer, char key) {
    if (buffer.empty()) {
        return false;
    }
    normalizeVietnameseNfc(buffer);
    if (key == '9') {
        if (buffer.back() == U'd') {
            buffer.back() = U'đ';
            return true;
        }
        const auto span = findLastSyllable(buffer);
        if (span && span->begin < span->onset_end && buffer[span->begin] == U'd') {
            buffer[span->begin] = U'đ';
            return true;
        }
        return false;
    }

    const auto span = findLastSyllable(buffer);
    if (!span) {
        return false;
    }

    const std::u32string tonePattern = baseSlice(buffer, span->medial_end, span->nucleus_end);
    if (key == '7') {
        if (tonePattern == U"uu") {
            return replaceWithSetPreserveTone(buffer, span->medial_end, 10);
        }
        if (tonePattern == U"uou" || tonePattern == U"ưou") {
            return replaceWithSetPreserveTone(buffer, span->medial_end + 1, 8);
        }
        if (tonePattern == U"uơu") {
            return replaceWithSetPreserveTone(buffer, span->medial_end, 10);
        }
    }

    std::optional<std::size_t> pos;
    std::size_t newSetIdx = 0;
    if (key == '6') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"aeo");
        if (!pos) {
            return false;
        }
        const char32_t base = baseChar(buffer[*pos]);
        if (base == U'a') {
            newSetIdx = 2;
        } else if (base == U'e') {
            newSetIdx = 4;
        } else if (base == U'o') {
            newSetIdx = 7;
        } else {
            return false;
        }
    } else if (key == '7') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"ou");
        if (!pos) {
            return false;
        }
        const char32_t base = baseChar(buffer[*pos]);
        if (base == U'o') {
            newSetIdx = 8;
        } else if (base == U'u') {
            newSetIdx = 10;
        } else {
            return false;
        }
    } else if (key == '8') {
        pos = rightmostNucleusCharWithBase(buffer, *span, U"a");
        newSetIdx = 1;
    } else {
        return false;
    }

    if (!pos) {
        return false;
    }
    return replaceWithSetPreserveTone(buffer, *pos, newSetIdx);
}

}  // namespace cbakey::core::vi_syllable
