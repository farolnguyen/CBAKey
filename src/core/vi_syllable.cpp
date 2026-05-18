#include "cbakey/core/vi_syllable.h"

#include <array>
#include <cwctype>
#include <locale.h>
#include <optional>
#include <string_view>
#include <vector>

namespace cbakey::core::vi_syllable {
namespace {

locale_t utf8CTypeLocale() {
    static locale_t loc = []() -> locale_t {
        static const char* const kNames[] = {"C.UTF-8", "C.utf8", "en_US.UTF-8", nullptr};
        for (std::size_t i = 0; kNames[i] != nullptr; ++i) {
            if (locale_t l = newlocale(LC_CTYPE_MASK, kNames[i], static_cast<locale_t>(0))) {
                return l;
            }
        }
        return static_cast<locale_t>(0);
    }();
    return loc;
}

wint_t utf8CTypeTowupper(wint_t wc) {
    if (locale_t loc = utf8CTypeLocale()) {
        return towupper_l(wc, loc);
    }
    return towupper(wc);
}

wint_t utf8CTypeTowlower(wint_t wc) {
    if (locale_t loc = utf8CTypeLocale()) {
        return towlower_l(wc, loc);
    }
    return towlower(wc);
}

int utf8CTypeIswupper(wint_t wc) {
    if (locale_t loc = utf8CTypeLocale()) {
        return iswupper_l(wc, loc);
    }
    return iswupper(wc);
}

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
    const auto tryChar = [&](char32_t c) -> bool {
        for (std::size_t si = 0; si < sets.size(); ++si) {
            for (std::size_t ti = 0; ti < sets[si].size(); ++ti) {
                const char32_t member = sets[si][ti];
                if (member == c) {
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
    };
    if (tryChar(ch)) {
        return true;
    }
    const std::wint_t w = static_cast<std::wint_t>(ch);
    if (w != static_cast<std::wint_t>(WEOF)) {
        const std::wint_t wl = utf8CTypeTowlower(w);
        if (wl != w) {
            return tryChar(static_cast<char32_t>(wl));
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

/// Maps a Vietnamese vowel (possibly with diacritics) to a single Latin letter used only for
/// \p selectToneOffset pattern matching. Without this, nucleus "oă" would become "o"+U+0103 and
/// would not match rules keyed as U"oa" (e.g. "hoặc").
char32_t tonePatternBucket(char32_t ch) {
    const char32_t b = baseChar(ch);
    switch (b) {
        case U'ă':
        case U'â':
            return U'a';
        case U'ê':
            return U'e';
        case U'ô':
        case U'ơ':
            return U'o';
        case U'ư':
            return U'u';
        default:
            return b;
    }
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

bool latinEq(char32_t a, char32_t b) {
    if (a == b) {
        return true;
    }
    const std::wint_t wa = static_cast<std::wint_t>(a);
    const std::wint_t wb = static_cast<std::wint_t>(b);
    if (wa == WEOF || wb == WEOF) {
        return false;
    }
    return utf8CTypeTowlower(wa) == utf8CTypeTowlower(wb);
}

bool startsWithLatinCaseFold(const std::u32string& s, std::u32string_view prefix) {
    if (s.size() < prefix.size()) {
        return false;
    }
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        if (!latinEq(s[i], prefix[i])) {
            return false;
        }
    }
    return true;
}

bool endsWithLatinCaseFold(const std::u32string& s, std::u32string_view suffix) {
    if (s.size() < suffix.size()) {
        return false;
    }
    const std::size_t offset = s.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        if (!latinEq(s[offset + i], suffix[i])) {
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
        if (startsWithLatinCaseFold(base, onset)) {
            return onset.size();
        }
    }
    return 0;
}

std::size_t longestCodaLength(const std::u32string& base) {
    static constexpr std::u32string_view kCodas[] = {U"ch", U"nh", U"ng", U"c", U"m", U"n", U"p", U"t"};
    for (std::u32string_view coda : kCodas) {
        if (endsWithLatinCaseFold(base, coda)) {
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

    if (onsetLen == 1 && latinEq(base[0], U'q') && coreBegin + 1 < coreEnd && latinEq(base[coreBegin], U'u') &&
        isVowelBase(base[coreBegin + 1])) {
        medialLen = 1;
    } else if (onsetLen == 1 && latinEq(base[0], U'g') && coreBegin + 1 < coreEnd && latinEq(base[coreBegin], U'i') &&
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

    // Standard Vietnamese has at most one non-ngang tone per syllable. Reject parses that stack
    // multiple toned vowels in the nucleus (e.g. typo "chủân" mis-read as one syllable), which
    // would confuse strip-diacritics and segmentation.
    std::size_t nonNgangToneCount = 0;
    for (std::size_t i = nucleusBegin; i < coreEnd; ++i) {
        std::size_t toneIdx = 0;
        if (!locateInToneSets(buffer[begin + i], nullptr, &toneIdx)) {
            continue;
        }
        if (toneIdx != 0) {
            ++nonNgangToneCount;
        }
    }
    if (nonNgangToneCount > 1) {
        return std::nullopt;
    }

    // Typo "chủân" / "chủấn" / NFD "chu\u0309a\u0302n": nucleus is u-bucket vowel with tone + a letter
    // from the **â** tone-set row (â ấ ầ …). That is not a legal single Vietnamese syllable (cf. "thuần"
    // where the leading u is toneless). Plain "a" row (without circumflex) is still allowed (e.g. "thúa").
    if (coreEnd - nucleusBegin == 2) {
        const char32_t v0 = buffer[begin + nucleusBegin];
        const char32_t v1 = buffer[begin + nucleusBegin + 1];
        std::size_t t0 = 0;
        if (locateInToneSets(v0, nullptr, &t0) && t0 != 0 && tonePatternBucket(v0) == U'u') {
            std::size_t si1 = 0;
            if (locateInToneSets(v1, &si1, nullptr) && toneSets()[si1][0] == U'â') {
                return std::nullopt;
            }
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

std::optional<std::vector<SyllableSpan>> segmentWholeBufferWithPreference(const std::u32string& buffer,
                                                                          bool minimizeSyllableCount) {
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

            bool better = false;
            if (minimizeSyllableCount) {
                better = !best || candidate.size() < best->size() ||
                         (candidate.size() == best->size() && candidate.back().begin > best->back().begin);
            } else {
                better = !best || candidate.size() > best->size() ||
                         (candidate.size() == best->size() && candidate.back().begin < best->back().begin);
            }
            if (better) {
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
        {U"ơi", 0, 0},   {U"ui", 0, 0},   {U"iu", 0, 0},   {U"ưi", 0, 0},   {U"ua", 0, 1},
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
    char32_t& ch = buffer[pos];
    const std::wint_t wch = static_cast<std::wint_t>(ch);
    const bool wasUpper = utf8CTypeIswupper(wch) != 0;
    std::size_t oldSetIdx = 0;
    std::size_t toneIdx = 0;
    if (!locateInToneSets(ch, &oldSetIdx, &toneIdx)) {
        return false;
    }
    (void)oldSetIdx;
    char32_t repl = toneSets()[setIdx][toneIdx];
    if (wasUpper) {
        repl = static_cast<char32_t>(utf8CTypeTowupper(static_cast<std::wint_t>(repl)));
    }
    ch = repl;
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

std::optional<std::vector<SyllableSpan>> segmentWholeBuffer(const std::u32string& buffer) {
    return segmentWholeBufferWithPreference(buffer, true);
}

std::optional<std::vector<SyllableSpan>> segmentWholeBufferPreferMaxSyllables(const std::u32string& buffer) {
    return segmentWholeBufferWithPreference(buffer, false);
}

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
    if (buffer.size() < 2) {
        return std::nullopt;
    }
    // Use minimize-syllable-count segmentation (fewest syllables, tiebreak: prefer
    // latest-starting last syllable).  If the optimal last syllable has no onset the
    // buffer is still ambiguous (e.g. "xina" → "xin"+"a" with no-onset "a") → nullopt.
    const auto segs = segmentWholeBufferWithPreference(buffer, /*minimizeSyllableCount=*/true);
    if (!segs || segs->size() < 2) {
        return std::nullopt;
    }
    const auto& last = segs->back();
    if (last.onset_end == last.begin) {
        return std::nullopt;
    }
    return StableComposeSplit{.committed_prefix_end = last.begin, .active_suffix = last};
}

bool normalizeVietnameseNfc(std::u32string& buffer) {
    bool any = false;
    // Multiple passes: one combining mark may need the previous char to already be merged
    // (e.g. NFD "u\u0309" -> ủ, then "â" from "a\u0302") so a single forward scan is not always enough.
    for (int pass = 0; pass < 32; ++pass) {
        std::u32string normalized;
        normalized.reserve(buffer.size());
        for (const char32_t ch : buffer) {
            if (!normalized.empty() && applyCombiningMark(normalized.back(), ch)) {
                continue;
            }
            normalized.push_back(ch);
        }
        if (normalized == buffer) {
            return any;
        }
        buffer = std::move(normalized);
        any = true;
    }
    return any;
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
        tonePattern.push_back(tonePatternBucket(buffer[i]));
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
    if (key == 'd') {
        const char32_t back = buffer.back();
        if (back == U'd' || back == U'D') {
            buffer.back() = (back == U'D') ? U'Đ' : U'đ';
            return true;
        }
    }

    const auto span = findLastSyllable(buffer);
    if (!span) {
        return false;
    }

    const std::u32string tonePattern = baseSlice(buffer, span->medial_end, span->nucleus_end);
    // ── Diphthong 'ươ'/'uô' toggle (Telex, size-2 nucleus) ──────────────────────
    if (span->nucleus_end - span->medial_end == 2) {
        if (key == 'w') {
            if (tonePattern == U"uo") {
                // 'uo' + 'w' → 'uơ' (step 1; normalizeTelexBuffer completes when coda added).
                return replaceWithSetPreserveTone(buffer, span->medial_end + 1, 8);
            }
            if (tonePattern == U"ươ") {
                // 'ươ' + 'w' → revert to plain 'uo' (strip both diacritics + tone).
                buffer[span->medial_end]     = U'u';
                buffer[span->medial_end + 1] = U'o';
                return true;
            }
            if (tonePattern == U"uô") {
                // 'uô' + 'w' → 'ươ' (u→ư, ô→ơ, preserve tone).
                bool c = replaceWithSetPreserveTone(buffer, span->medial_end,     10);
                c     |= replaceWithSetPreserveTone(buffer, span->medial_end + 1,  8);
                return c;
            }
        }
        if (key == 'o' && tonePattern == U"ươ") {
            // 'ươ' + 'o' → swap to 'uô' (ư→u, ơ→ô, preserve tone).
            // e.g. 'đượ c' + 'o' → 'đuộ c'; user then presses 's' → 'đuốc'.
            bool c = replaceWithSetPreserveTone(buffer, span->medial_end,     9); // ư → u
            c     |= replaceWithSetPreserveTone(buffer, span->medial_end + 1, 7); // ơ → ô
            return c;
        }
    }
    // ─────────────────────────────────────────────────────────────────────────────

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

bool normalizeVniUoTransform(std::u32string& buffer) {
    // After VNI '7' transforms 'uo' → 'uơ' (only 'o' → 'ơ'), this function
    // completes the transform by converting 'u' → 'ư' — but ONLY when:
    //   • the nucleus is exactly "uơ" (size == 2, NOT "uơi"/"uơu"/etc.), AND
    //   • there is a coda (i.e. not an open syllable like "thuơ" → "thuở").
    // This avoids breaking "cuoi717" → "cưới" (nucleus "uơi", size 3)
    // and "thuơ" → "thuở" (no coda).
    normalizeVietnameseNfc(buffer);
    const auto span = findLastSyllable(buffer);
    if (!span) return false;

    const std::u32string tonePattern = baseSlice(buffer, span->medial_end, span->nucleus_end);
    const bool hasCoda = span->nucleus_end < span->end;

    if (tonePattern.size() != 2 || !hasCoda) return false;
    if (!startsWith(tonePattern, U"uơ")) return false;

    return replaceWithSetPreserveTone(buffer, span->medial_end, 10);  // u → ư
}

bool removeVietnameseDiacritics(std::u32string& buffer) {
    normalizeVietnameseNfc(buffer);

    const auto stripOne = [](char32_t& ch) -> bool {
        if (ch == U'đ') {
            ch = U'd';
            return true;
        }
        if (ch == U'Đ') {
            ch = U'D';
            return true;
        }
        const std::wint_t w = static_cast<std::wint_t>(ch);
        const bool wasUpper = utf8CTypeIswupper(w) != 0;
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
        if (normalized == ch) {
            return false;
        }
        if (wasUpper && normalized >= U'a' && normalized <= U'z') {
            ch = static_cast<char32_t>(normalized - U'a' + U'A');
        } else {
            ch = normalized;
        }
        return true;
    };

    std::optional<SyllableSpan> target;
    if (const auto last = findLastSyllable(buffer)) {
        target = *last;
    }

    bool changed = false;
    if (target) {
        for (std::size_t i = target->begin; i < target->end; ++i) {
            if (stripOne(buffer[i])) {
                changed = true;
            }
        }
        return changed;
    }

    for (char32_t& ch : buffer) {
        if (stripOne(ch)) {
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
        const char32_t back = buffer.back();
        if (back == U'd' || back == U'D') {
            buffer.back() = (back == U'D') ? U'Đ' : U'đ';
            return true;
        }
        const auto span = findLastSyllable(buffer);
        if (span && span->begin < span->onset_end) {
            const char32_t onset = buffer[span->begin];
            if (onset == U'd' || onset == U'D') {
                buffer[span->begin] = (onset == U'D') ? U'Đ' : U'đ';
                return true;
            }
        }
        return false;
    }

    const auto span = findLastSyllable(buffer);
    if (!span) {
        return false;
    }

    const std::u32string tonePattern = baseSlice(buffer, span->medial_end, span->nucleus_end);
    const std::size_t nucleusLen = span->nucleus_end - span->medial_end;

    // ── Diphthong 'ươ'/'uô' toggle (size-2 nucleus only) ────────────────────────
    if (nucleusLen == 2) {
        if (tonePattern == U"uô") {
            if (key == '7') {
                // 'uô' + '7' → 'ươ': swap diacritic (ô→ơ) AND convert u→ư (preserve tone).
                bool c = replaceWithSetPreserveTone(buffer, span->medial_end,     10); // u  → ư
                c     |= replaceWithSetPreserveTone(buffer, span->medial_end + 1,  8); // ô → ơ
                return c;
            }
        }
        if (tonePattern == U"ươ") {
            if (key == '7') {
                // 'ươ' + '7' → revert to plain 'uo' (strip both diacritics + tone).
                buffer[span->medial_end]     = U'u';
                buffer[span->medial_end + 1] = U'o';
                return true;
            }
            if (key == '6') {
                // 'ươ' + '6' → swap to 'uô' (ư→u, ơ→ô, preserve tone).
                // e.g. 'đượ c' + '6' → 'đuộ c'; user then presses '1' → 'đuốc'.
                bool c = replaceWithSetPreserveTone(buffer, span->medial_end,     9); // ư → u
                c     |= replaceWithSetPreserveTone(buffer, span->medial_end + 1, 7); // ơ → ô
                return c;
            }
        }
    }
    // ─────────────────────────────────────────────────────────────────────────────

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
