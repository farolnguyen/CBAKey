#include "cbakey/core/engine.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <cwctype>
#include <locale.h>
#include <optional>
#include <string>
#include <vector>

#include "cbakey/core/user_dict.h"
#include "cbakey/core/vi_syllable.h"

namespace cbakey::core {

namespace {

constexpr std::size_t kToneCount = 6;  // ngang, sac, huyen, hoi, nga, nang

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

std::u32string decodeUtf8(const std::string& input) {
    std::u32string output;
    for (std::size_t i = 0; i < input.size();) {
        const unsigned char c = static_cast<unsigned char>(input[i]);
        if ((c & 0x80) == 0) {
            output.push_back(static_cast<char32_t>(c));
            ++i;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < input.size()) {
            const char32_t cp = ((c & 0x1F) << 6) |
                                (static_cast<unsigned char>(input[i + 1]) & 0x3F);
            output.push_back(cp);
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < input.size()) {
            const char32_t cp = ((c & 0x0F) << 12) |
                                ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6) |
                                (static_cast<unsigned char>(input[i + 2]) & 0x3F);
            output.push_back(cp);
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < input.size()) {
            const char32_t cp = ((c & 0x07) << 18) |
                                ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12) |
                                ((static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6) |
                                (static_cast<unsigned char>(input[i + 3]) & 0x3F);
            output.push_back(cp);
            i += 4;
        } else {
            ++i;
        }
    }
    return output;
}

void stripComposeIgnorableCodePoints(std::u32string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](char32_t c) {
                switch (c) {
                    case U'\u200B':  // ZWSP (common in web editors / rich text)
                    case U'\u200C':  // ZWNJ
                    case U'\u200D':  // ZWJ
                    case U'\uFEFF':  // BOM
                        return true;
                    default:
                        return false;
                }
            }),
            s.end());
}

std::u32string decodeUtf8Normalized(const std::string& input) {
    std::u32string output = decodeUtf8(input);
    stripComposeIgnorableCodePoints(output);
    vi_syllable::normalizeVietnameseNfc(output);
    return output;
}

std::string encodeUtf8(std::u32string input) {
    vi_syllable::normalizeVietnameseNfc(input);
    std::string output;
    for (const char32_t cp : input) {
        if (cp <= 0x7F) {
            output.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            output.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            output.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            output.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            output.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            output.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            output.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return output;
}

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

bool applyToneAt(std::u32string& buffer, std::size_t pos, std::size_t toneIndex) {
    char32_t& ch = buffer[pos];
    const std::wint_t wch = static_cast<std::wint_t>(ch);
    const bool wasUpper = utf8CTypeIswupper(wch) != 0;
    const auto& sets = toneSets();
    for (const auto& set : sets) {
        for (std::size_t k = 0; k < kToneCount; ++k) {
            if (set[k] == ch) {
                char32_t repl = set[toneIndex];
                if (wasUpper) {
                    repl = static_cast<char32_t>(utf8CTypeTowupper(static_cast<std::wint_t>(repl)));
                }
                ch = repl;
                return true;
            }
        }
    }
    if (wasUpper) {
        const std::wint_t wl = utf8CTypeTowlower(wch);
        if (wl != wch) {
            const char32_t lowerCh = static_cast<char32_t>(wl);
            for (const auto& set : sets) {
                for (std::size_t k = 0; k < kToneCount; ++k) {
                    if (set[k] == lowerCh) {
                        char32_t repl = set[toneIndex];
                        repl = static_cast<char32_t>(utf8CTypeTowupper(static_cast<std::wint_t>(repl)));
                        ch = repl;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool applyTone(std::u32string& buffer, char toneKey) {
    std::size_t toneIndex = 0;
    switch (toneKey) {
        case 's':
            toneIndex = 1;
            break;
        case 'f':
            toneIndex = 2;
            break;
        case 'r':
            toneIndex = 3;
            break;
        case 'x':
            toneIndex = 4;
            break;
        case 'j':
            toneIndex = 5;
            break;
        default:
            return false;
    }

    const auto pos = vi_syllable::selectToneVowelIndex(buffer);
    if (!pos) {
        return false;
    }
    return applyToneAt(buffer, *pos, toneIndex);
}

bool applyToneVni(std::u32string& buffer, char key) {
    std::size_t toneIndex = 0;
    switch (key) {
        case '1':
            toneIndex = 1;
            break;
        case '2':
            toneIndex = 2;
            break;
        case '3':
            toneIndex = 3;
            break;
        case '4':
            toneIndex = 4;
            break;
        case '5':
            toneIndex = 5;
            break;
        default:
            return false;
    }

    const auto pos = vi_syllable::selectToneVowelIndex(buffer);
    if (!pos) {
        return false;
    }
    return applyToneAt(buffer, *pos, toneIndex);
}

bool isTelexRepeatableKey(char key) {
    switch (key) {
        case 'a':
        case 'd':
        case 'e':
        case 'f':
        case 'j':
        case 'o':
        case 'r':
        case 's':
        case 'w':
        case 'x':
            return true;
        default:
            return false;
    }
}

bool isVniRepeatableKey(char key) {
    return key >= '1' && key <= '5';
}

bool isTelexEscapableKey(char key) {
    switch (key) {
        case '\\':
        case 'a':
        case 'd':
        case 'e':
        case 'f':
        case 'j':
        case 'o':
        case 'r':
        case 's':
        case 'w':
        case 'x':
        case 'z':
            return true;
        default:
            return false;
    }
}

bool isVniEscapableKey(char key) {
    return key == '\\' || (key >= '1' && key <= '9');
}

}  // namespace

namespace {

/// Resolve the user dictionary path from config or XDG default.
std::string resolveUserDictPath(const cbakey::config::RuntimeConfig& cfg) {
    if (!cfg.userDictPath.empty()) {
        return cfg.userDictPath;
    }
    // XDG_CONFIG_HOME / cbakey / user_dict.json
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    std::string base;
    if (xdg && xdg[0] != '\0') {
        base = xdg;
    } else {
        const char* home = std::getenv("HOME");
        base = home ? std::string(home) + "/.config" : "/tmp";
    }
    return base + "/cbakey/user_dict.json";
}

}  // namespace

Engine::Engine(cbakey::config::RuntimeConfig config)
    : config_(std::move(config)),
      userDict_(config_.enableUserDictionary
                    ? UserDict::loadFromFile(resolveUserDictPath(config_))
                    : UserDict{}) {}

void Engine::clearRepeatTransformState() {
    repeatTransformState_ = RepeatTransformState{};
}

void Engine::clearPendingLiteralEscape() {
    pendingLiteralEscape_ = false;
}

bool Engine::isAsciiSeparatorCommit(char ch) {
    const auto u = static_cast<unsigned char>(ch);
    return u < 128 && std::ispunct(u) != 0;
}

std::string Engine::takeCompositionForCommit() {
    std::string out = std::move(preeditBuffer_);
    preeditHistory_.clear();
    clearRepeatTransformState();
    clearPendingLiteralEscape();
    return out;
}

ProcessResult Engine::processKey(const KeyEvent& event) {
    if (isToggleHotkey(event)) {
        mode_ = (mode_ == InputMode::English) ? InputMode::Vietnamese : InputMode::English;
        clearRepeatTransformState();
        clearPendingLiteralEscape();
        return ProcessResult{.preedit = "", .commit = "", .consumed = true};
    }

    // Common application shortcuts (copy/paste, word navigation, etc.) should
    // bypass IME processing unless they are our explicit toggle hotkey.
    if (event.ctrl || event.alt) {
        return ProcessResult{};
    }

    return mode_ == InputMode::Vietnamese ? processVietnameseKey(event) : processEnglishKey(event);
}

void Engine::setInputMode(InputMode mode) {
    mode_ = mode;
    clearRepeatTransformState();
    clearPendingLiteralEscape();
}

InputMode Engine::inputMode() const {
    return mode_;
}

void Engine::clearState() {
    preeditBuffer_.clear();
    preeditHistory_.clear();
    clearRepeatTransformState();
    clearPendingLiteralEscape();
}

bool Engine::isToggleHotkey(const KeyEvent& event) const {
    return event.ctrl && event.alt && (event.key == 'z' || event.key == 'Z');
}

ProcessResult Engine::processVietnameseKey(const KeyEvent& event) {
    auto commitWithSuffix = [this](std::string suffix) -> ProcessResult {
        ProcessResult r;
        if (preeditBuffer_.empty()) {
            return r;
        }
        // M8.2: user dict static expansion.
        // Dict always takes priority — if the user put a trigger in the dict they want it
        // to expand. Expansion fires on word-boundary keys (space, enter, tab).
        if (config_.enableUserDictionary && !userDict_.empty()) {
            if (const UserDictEntry* entry = userDict_.lookup(preeditBuffer_)) {
                r.commit = entry->expansion + suffix;
                preeditBuffer_.clear();
                preeditHistory_.clear();
                clearRepeatTransformState();
                clearPendingLiteralEscape();
                r.consumed = true;
                return r;
            }
        }
        r.commit = preeditBuffer_ + std::move(suffix);
        preeditBuffer_.clear();
        preeditHistory_.clear();
        clearRepeatTransformState();
        clearPendingLiteralEscape();
        r.consumed = true;
        return r;
    };

    const auto materializeLiteralChar = [this](char literal) {
        preeditHistory_.push_back(preeditBuffer_);
        std::u32string decoded = decodeUtf8Normalized(preeditBuffer_);
        decoded.push_back(static_cast<unsigned char>(literal));
        preeditBuffer_ = encodeUtf8(decoded);
    };

    const auto deleteVisibleTail = [this]() -> ProcessResult {
        std::u32string decoded = decodeUtf8Normalized(preeditBuffer_);
        if (decoded.empty()) {
            clearRepeatTransformState();
            clearPendingLiteralEscape();
            return ProcessResult{.preedit = "", .commit = "", .consumed = false};
        }
        decoded.pop_back();
        preeditBuffer_ = encodeUtf8(decoded);
        preeditHistory_.clear();
        clearRepeatTransformState();
        clearPendingLiteralEscape();
        return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
    };

    const auto maybeAutoCommitStablePrefix =
        [this](const std::u32string& decoded) -> std::optional<ProcessResult> {
        const auto split = vi_syllable::findStableComposeSplit(decoded);
        if (!split) {
            return std::nullopt;
        }

        const std::string committedPrefix =
            encodeUtf8(decoded.substr(0, split->committed_prefix_end));
        const std::string activeSuffix =
            encodeUtf8(decoded.substr(split->committed_prefix_end));
        if (committedPrefix.empty() || activeSuffix.empty()) {
            return std::nullopt;
        }

        std::vector<std::string> rebasedHistory;
        rebasedHistory.reserve(preeditHistory_.size());
        for (const std::string& state : preeditHistory_) {
            if (state.size() < committedPrefix.size() ||
                state.compare(0, committedPrefix.size(), committedPrefix) != 0) {
                continue;
            }
            rebasedHistory.push_back(state.substr(committedPrefix.size()));
        }
        preeditHistory_ = std::move(rebasedHistory);

        if (repeatTransformState_.active) {
            if (repeatTransformState_.buffer_before.size() >= committedPrefix.size() &&
                repeatTransformState_.buffer_before.compare(0, committedPrefix.size(), committedPrefix) == 0) {
                repeatTransformState_.buffer_before =
                    repeatTransformState_.buffer_before.substr(committedPrefix.size());
            } else {
                clearRepeatTransformState();
            }
        }

        preeditBuffer_ = activeSuffix;
        return ProcessResult{.preedit = preeditBuffer_, .commit = committedPrefix, .consumed = true};
    };

    if (config_.method == cbakey::core::InputMethod::Vni && event.key_from_keypad &&
        event.aux == KeyAux::None && event.key != '\0' &&
        std::isdigit(static_cast<unsigned char>(event.key)) != 0 && !preeditBuffer_.empty()) {
        ProcessResult r;
        r.commit = takeCompositionForCommit();
        r.consumed = true;
        r.forwardOriginalKey = true;
        return r;
    }

    if (pendingLiteralEscape_) {
        const char pendingRaw = event.key;
        const char pendingKey = static_cast<char>(std::tolower(static_cast<unsigned char>(pendingRaw)));
        clearRepeatTransformState();
        const bool escapable =
            config_.method == cbakey::core::InputMethod::Telex ? isTelexEscapableKey(pendingKey)
                                                               : isVniEscapableKey(pendingKey);
        if (event.aux == KeyAux::None && pendingRaw != '\0' && escapable) {
            materializeLiteralChar(pendingRaw);
            clearPendingLiteralEscape();
            return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
        }
        materializeLiteralChar('\\');
        clearPendingLiteralEscape();
    }

    switch (event.aux) {
        case KeyAux::None:
            break;
        case KeyAux::Left:
        case KeyAux::Right:
        case KeyAux::Up:
        case KeyAux::Down:
        case KeyAux::Home:
        case KeyAux::End:
            if (!preeditBuffer_.empty()) {
                ProcessResult r;
                r.commit = preeditBuffer_;
                preeditBuffer_.clear();
                preeditHistory_.clear();
                clearRepeatTransformState();
                clearPendingLiteralEscape();
                r.consumed = true;
                r.forwardOriginalKey = true;
                return r;
            }
            clearRepeatTransformState();
            clearPendingLiteralEscape();
            return ProcessResult{};
        case KeyAux::DeleteForward:
            if (!preeditBuffer_.empty()) {
                return deleteVisibleTail();
            }
            clearRepeatTransformState();
            clearPendingLiteralEscape();
            return ProcessResult{};
        case KeyAux::Enter:
            return commitWithSuffix("\n");
        case KeyAux::Tab:
            return commitWithSuffix("\t");
    }

    if (event.aux == KeyAux::None && event.key == '\t') {
        return commitWithSuffix("\t");
    }
    if (event.aux == KeyAux::None && event.key == '\n') {
        return commitWithSuffix("\n");
    }

    if (event.key == '\b') {
        if (!preeditBuffer_.empty()) {
            return deleteVisibleTail();
        }

        clearRepeatTransformState();
        clearPendingLiteralEscape();
        return ProcessResult{.preedit = "", .commit = "", .consumed = false};
    }

    if (event.aux == KeyAux::None && event.key == '\\') {
        clearRepeatTransformState();
        pendingLiteralEscape_ = true;
        return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
    }

    if (event.key == ' ') {
        return commitWithSuffix(" ");
    }

    if (event.aux == KeyAux::None && event.key != '\0' &&
        isAsciiSeparatorCommit(static_cast<char>(event.key))) {
        if (!preeditBuffer_.empty()) {
            return commitWithSuffix(std::string(1, static_cast<char>(event.key)));
        }
        clearRepeatTransformState();
        clearPendingLiteralEscape();
        return ProcessResult{};
    }

    const char raw = event.key;
    const char key = static_cast<char>(std::tolower(static_cast<unsigned char>(raw)));
    preeditHistory_.push_back(preeditBuffer_);
    const std::string bufferBefore = preeditBuffer_;

    std::u32string decoded = decodeUtf8Normalized(preeditBuffer_);
    if ((config_.method == cbakey::core::InputMethod::Telex && key == 'z') ||
        (config_.method == cbakey::core::InputMethod::Vni && key == '0')) {
        clearRepeatTransformState();
        const bool removed =
            config_.method == cbakey::core::InputMethod::Telex ? vi_syllable::removeTelexDiacritics(decoded)
                                                               : vi_syllable::removeVniDiacritics(decoded);
        if (removed) {
            preeditBuffer_ = encodeUtf8(decoded);
            clearPendingLiteralEscape();
            if (const auto split = maybeAutoCommitStablePrefix(decoded)) {
                return *split;
            }
            return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
        }
        decoded = decodeUtf8Normalized(preeditBuffer_);
    }

    if (repeatTransformState_.active && repeatTransformState_.key == key &&
        ((config_.method == cbakey::core::InputMethod::Telex && isTelexRepeatableKey(key)) ||
         (config_.method == cbakey::core::InputMethod::Vni && isVniRepeatableKey(key)))) {
        std::u32string reverted = decodeUtf8Normalized(repeatTransformState_.buffer_before);
        reverted.push_back(static_cast<unsigned char>(raw));
        if (config_.method == cbakey::core::InputMethod::Telex) {
            vi_syllable::normalizeTelexBuffer(reverted);
        }
        preeditBuffer_ = encodeUtf8(reverted);
        clearRepeatTransformState();
        clearPendingLiteralEscape();
        if (const auto split = maybeAutoCommitStablePrefix(reverted)) {
            return *split;
        }
        return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
    }
    bool transformed = false;
    if (config_.method == cbakey::core::InputMethod::Telex) {
        if (key == 's' || key == 'f' || key == 'r' || key == 'x' || key == 'j') {
            transformed = applyTone(decoded, key);
        } else {
            transformed = vi_syllable::applyTelexTransform(decoded, key);
        }
    } else {
        if (key >= '1' && key <= '5') {
            transformed = applyToneVni(decoded, key);
        } else {
            transformed = vi_syllable::applyVniTransform(decoded, key);
        }
    }

    if (transformed) {
        preeditBuffer_ = encodeUtf8(decoded);
        if ((config_.method == cbakey::core::InputMethod::Telex && isTelexRepeatableKey(key)) ||
            (config_.method == cbakey::core::InputMethod::Vni && isVniRepeatableKey(key))) {
            repeatTransformState_.active = true;
            repeatTransformState_.key = key;
            repeatTransformState_.buffer_before = bufferBefore;
        } else {
            clearRepeatTransformState();
        }
        clearPendingLiteralEscape();
        if (const auto split = maybeAutoCommitStablePrefix(decoded)) {
            return *split;
        }
        return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
    }

    clearRepeatTransformState();
    clearPendingLiteralEscape();
    decoded.push_back(static_cast<unsigned char>(raw));
    if (config_.method == cbakey::core::InputMethod::Telex) {
        vi_syllable::normalizeTelexBuffer(decoded);
    }
    preeditBuffer_ = encodeUtf8(decoded);
    if (const auto split = maybeAutoCommitStablePrefix(decoded)) {
        return *split;
    }
    return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
}

ProcessResult Engine::processEnglishKey(const KeyEvent& event) {
    clearRepeatTransformState();
    clearPendingLiteralEscape();
    if (event.aux != KeyAux::None) {
        return ProcessResult{};
    }
    if (event.key == '\0') {
        return ProcessResult{.preedit = "", .commit = "", .consumed = false};
    }

    return ProcessResult{.preedit = "", .commit = std::string(1, event.key), .consumed = true};
}

void Engine::seedPreeditForCommittedRewrite(std::string utf8) {
    clearState();
    preeditBuffer_ = std::move(utf8);
}

std::optional<std::string> Engine::tryRewriteCommittedSyllable(const cbakey::config::RuntimeConfig& config,
                                                               const std::string& token_utf8,
                                                               const KeyEvent& event) {
    if (token_utf8.empty() || token_utf8.size() > 96) {
        return std::nullopt;
    }
    const std::u32string dec = decodeUtf8Normalized(token_utf8);
    if (dec.empty()) {
        return std::nullopt;
    }
    const auto span = vi_syllable::findLastSyllable(dec);
    if (!span || span->begin != 0 || span->end != dec.size()) {
        return std::nullopt;
    }
    Engine scratch(config);
    scratch.seedPreeditForCommittedRewrite(token_utf8);
    const ProcessResult r = scratch.processKey(event);
    if (!r.consumed) {
        return std::nullopt;
    }
    if (!r.commit.empty()) {
        return std::nullopt;
    }
    if (r.preedit.empty()) {
        return std::nullopt;
    }
    if (config.method == cbakey::core::InputMethod::Vni && event.key >= '1' && event.key <= '5') {
        if (r.preedit.size() == token_utf8.size() + 1 &&
            r.preedit.compare(0, token_utf8.size(), token_utf8) == 0 &&
            r.preedit.back() == event.key) {
            return std::nullopt;
        }
    }
    if (r.preedit == token_utf8) {
        return std::nullopt;
    }
    return r.preedit;
}

}  // namespace cbakey::core
