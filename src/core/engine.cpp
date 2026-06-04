#include "farolkey/core/engine.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cwctype>
#include <locale.h>
#include <optional>
#include <string>
#include <vector>

#include "farolkey/core/user_dict.h"
#include "farolkey/core/vi_syllable.h"

namespace farolkey::core {

namespace {

// ---------------------------------------------------------------------------
// M15: Smart Templates — parametric expansion via farolkey-template subprocess
// ---------------------------------------------------------------------------

/// Shell-escape a string for single-quote wrapping: ' → '\''
static std::string shellEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    return out;
}

/// Call `farolkey-template expand --mode <mode> '<trigger>'`.
/// Returns the rendered template text, or empty string if no match / error.
/// Requires the "**" activation prefix so normal words never trigger templates
/// accidentally (e.g. "update" contains "date" but lacks "**").
static std::string tryParametricExpand(const std::string& input, const std::string& mode) {
    // Only activate when the user explicitly wrapped the trigger in [brackets].
    // e.g. [date] or [5++] — prevents accidental expansion of words that happen
    // to contain a template pattern (e.g. "update" containing "date").
    if (input.size() < 3 || input.front() != '[' || input.back() != ']') {
        return {};
    }
    const std::string trigger = input.substr(1, input.size() - 2);
    if (trigger.empty()) return {};

    const std::string cmd =
        "farolkey-template expand --mode " + mode +
        " '" + shellEscape(trigger) + "' 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return {};

    std::string result;
    char buf[512];
    while (std::fgets(buf, sizeof(buf), pipe)) {
        result += buf;
    }

    const int ret = pclose(pipe);
    if (ret != 0) return {};  // exit 1 = no matching template

    return result;
}

// ---------------------------------------------------------------------------

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

bool applyToneViqr(std::u32string& buffer, char key) {
    std::size_t toneIndex = 0;
    switch (key) {
        case '\'': toneIndex = 1; break;  // sắc
        case '`':  toneIndex = 2; break;  // huyền
        case '?':  toneIndex = 3; break;  // hỏi
        case '~':  toneIndex = 4; break;  // ngã
        case '.':  toneIndex = 5; break;  // nặng
        default:   return false;
    }
    const auto pos = vi_syllable::selectToneVowelIndex(buffer);
    if (!pos) return false;
    return applyToneAt(buffer, *pos, toneIndex);
}

bool isViqrRepeatableKey(char key) {
    switch (key) {
        case '^': case '(': case '+':
        case '\'': case '`': case '?': case '~': case '.':
        case 'd':
            return true;
        default:
            return false;
    }
}

bool isViqrStarRepeatableKey(char key) {
    switch (key) {
        case '^': case '(': case '*':  // '*' instead of '+' for horn
        case '\'': case '`': case '?': case '~': case '.':
        case 'd':
            return true;
        default:
            return false;
    }
}

bool isSimpleTelexRepeatableKey(char key) {
    // Only tone keys are special in Simple Telex.
    switch (key) {
        case 's': case 'f': case 'r': case 'x': case 'j':
            return true;
        default:
            return false;
    }
}

bool isMicrosoftRepeatableKey(char key) {
    switch (key) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 's': case 'j':
            return true;
        default:
            return false;
    }
}

bool isTocKyRepeatableKey(char key) {
    return key >= '1' && key <= '5';  // same as VNI tone keys
}

// Returns true if the decoded buffer contains at least one Vietnamese vowel.
// Used by Tốc ký to distinguish initial (onset) vs final (coda) shortcut context.
bool hasVietnameseVowel(const std::u32string& buf) {
    for (char32_t c : buf) {
        if (c == U'a' || c == U'e' || c == U'i' || c == U'o' || c == U'u' || c == U'y')
            return true;
        // All precomposed Vietnamese vowels are in U+00C0..U+1EFF.
        // Exclude Đ (U+0110) and đ (U+0111), the only Vietnamese precomposed consonants.
        if (c >= U'À' && c != U'Đ' && c != U'đ')
            return true;
    }
    return false;
}

}  // namespace

namespace {

/// Resolve the user dictionary path from config or XDG default.
std::string resolveUserDictPath(const farolkey::config::RuntimeConfig& cfg) {
    if (!cfg.userDictPath.empty()) {
        return cfg.userDictPath;
    }
    // XDG_CONFIG_HOME / farolkey / user_dict.json
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    std::string base;
    if (xdg && xdg[0] != '\0') {
        base = xdg;
    } else {
        const char* home = std::getenv("HOME");
        base = home ? std::string(home) + "/.config" : "/tmp";
    }
    return base + "/farolkey/user_dict.json";
}

}  // namespace

Engine::Engine(farolkey::config::RuntimeConfig config)
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
    // EN mode preeditBuffer_ is a word-tracking buffer only — each character
    // was already committed individually via commitString.  Returning it here
    // would cause a second commit (double-text) when the IM is reset, e.g.
    // Firefox/fcitx4 calls XIM ResetIC after passing a native key event
    // (like Backspace) back to the app, which triggers our reset() handler.
    if (mode_ == InputMode::English) {
        preeditBuffer_.clear();
        preeditHistory_.clear();
        clearRepeatTransformState();
        clearPendingLiteralEscape();
        return "";
    }
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
    // If preedit is active, commit it first so the app works on complete text
    // (e.g. Ctrl+A selects all including the just-committed word).
    if (event.ctrl || event.alt) {
        if (!preeditBuffer_.empty()) {
            ProcessResult r;
            r.commit   = takeCompositionForCommit();
            r.consumed = false;   // let the Ctrl/Alt shortcut reach the app
            return r;
        }
        return ProcessResult{};
    }

    return mode_ == InputMode::Vietnamese ? processVietnameseKey(event) : processEnglishKey(event);
}

void Engine::setInputMode(InputMode mode) {
    if (mode != mode_) {
        // Clear any pending preedit so it doesn't leak into the new mode.
        preeditBuffer_.clear();
        preeditHistory_.clear();
    }
    mode_ = mode;
    clearRepeatTransformState();
    clearPendingLiteralEscape();
}

InputMode Engine::inputMode() const {
    return mode_;
}

void Engine::setPasswordField(bool isPassword) {
    passwordField_ = isPassword;
}

bool Engine::isPasswordField() const {
    return passwordField_;
}

const UserDictEntry* Engine::lookupEnglishAbbrev(const std::string& trigger) const {
    if (!config_.enableUserDictionary || userDict_.empty() || passwordField_) return nullptr;
    const UserDictEntry* e = userDict_.lookup(trigger);
    if (!e) return nullptr;
    if (e->abbrev_mode != AbbrevMode::En && e->abbrev_mode != AbbrevMode::Both) return nullptr;
    return e;
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
        // Normalize before commit: converts intermediate states like "đuợc" → "được"
        // (normalizeTelexBuffer only runs on keypress, not at commit time).
        if (config_.method == farolkey::core::InputMethod::Telex) {
            auto decoded = decodeUtf8Normalized(preeditBuffer_);
            vi_syllable::normalizeTelexBuffer(decoded);
            preeditBuffer_ = encodeUtf8(decoded);
        }
        // M17.3: Smart Tone Normalization — move misplaced tones and strip invalid glide diacritics.
        // Runs for both Telex and VNI (e.g. "họăc" → "hoặc", "hơặc" → "hoặc").
        {
            auto decoded = decodeUtf8Normalized(preeditBuffer_);
            if (const auto span = vi_syllable::findLastSyllable(decoded)) {
                if (vi_syllable::normalizeSyllableTonePlacement(decoded, *span))
                    preeditBuffer_ = encodeUtf8(decoded);
            }
        }
        // M8.2/M13: user dict / abbreviation expansion.
        // Expansion fires on word-boundary keys (space, enter, tab).
        // In Vietnamese mode only Vi/Both entries expand; password fields are skipped.
        if (config_.enableUserDictionary && !userDict_.empty() && !passwordField_) {
            if (const UserDictEntry* entry = userDict_.lookup(preeditBuffer_)) {
                const bool modeOk = entry->abbrev_mode == AbbrevMode::Vi ||
                                    entry->abbrev_mode == AbbrevMode::Both;
                if (modeOk) {
                    r.commit = entry->expansion + suffix;
                    preeditBuffer_.clear();
                    preeditHistory_.clear();
                    clearRepeatTransformState();
                    clearPendingLiteralEscape();
                    r.consumed = true;
                    return r;
                }
            }
        }
        // M15: Smart Templates — try parametric expansion after exact-match fails.
        if (config_.enableSmartTemplates && !passwordField_) {
            const std::string expanded = tryParametricExpand(preeditBuffer_, "vi");
            if (!expanded.empty()) {
                r.commit = expanded;  // template defines its own output; no suffix
                preeditBuffer_.clear();
                preeditHistory_.clear();
                clearRepeatTransformState();
                clearPendingLiteralEscape();
                r.consumed = true;
                r.smartTemplateExpansion = true;
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

    // M17.4: Realtime tone normalization — fires after every preedit update.
    // Moves misplaced tones and strips invalid glide diacritics in the oaGlide pattern.
    // Modifies decoded in-place so maybeAutoCommitStablePrefix sees the corrected form.
    const auto applyPreeditNormalize = [&](std::u32string& dec) {
        if (const auto span = vi_syllable::findLastSyllable(dec)) {
            if (vi_syllable::normalizeSyllableTonePlacement(dec, *span)) {
                preeditBuffer_ = encodeUtf8(dec);
            }
        }
    };

    // VNI + numpad digit: digits on the numpad should never be treated as VNI
    // tone/diacritic modifiers — they must always produce the literal digit.
    // Commit any pending preedit first, then forward the key to the app.
    if ((config_.method == farolkey::core::InputMethod::Vni ||
         config_.method == farolkey::core::InputMethod::Microsoft ||
         config_.method == farolkey::core::InputMethod::TocKy) &&
        event.key_from_keypad && event.aux == KeyAux::None && event.key != '\0' &&
        std::isdigit(static_cast<unsigned char>(event.key)) != 0) {
        ProcessResult r;
        if (!preeditBuffer_.empty()) {
            // Commit pending preedit, then forward the numpad key explicitly.
            // forwardOriginalKey works here because there is existing state to flush.
            r.commit = takeCompositionForCommit();
            r.consumed = true;
            r.forwardOriginalKey = true;
        } else {
            // No preedit: let the key pass through fcitx5 naturally (consumed=false).
            // forwardKey(KP_1) strips Num Lock state → app sees navigation key instead
            // of digit; the natural path preserves the full key context.
            r.consumed = false;
        }
        return r;
    }

    if (pendingLiteralEscape_) {
        const char pendingRaw = event.key;
        const char pendingKey = static_cast<char>(std::tolower(static_cast<unsigned char>(pendingRaw)));
        clearRepeatTransformState();
        const bool escapable =
            config_.method == farolkey::core::InputMethod::Telex ? isTelexEscapableKey(pendingKey)
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

    // VIQR / VIQR* use '\' as remove-diacritics key — handled below.
    if (event.aux == KeyAux::None && event.key == '\\' &&
        config_.method != farolkey::core::InputMethod::Viqr &&
        config_.method != farolkey::core::InputMethod::ViqrStar) {
        clearRepeatTransformState();
        pendingLiteralEscape_ = true;
        return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
    }

    if (event.key == ' ') {
        return commitWithSuffix(" ");
    }

    // VIQR / VIQR* special keys are ASCII punctuation but must reach the
    // transform/remove-diacritics path when preedit is active.
    // VIQR uses '+' for horn; VIQR* uses '*' for horn — each bypasses only its own key.
    const bool isViqrMethod    = config_.method == farolkey::core::InputMethod::Viqr;
    const bool isViqrStarMethod = config_.method == farolkey::core::InputMethod::ViqrStar;
    const char viqrKey = static_cast<char>(event.key);
    const bool viqrBypass = (isViqrMethod || isViqrStarMethod) &&
                            !preeditBuffer_.empty() && [isViqrMethod, isViqrStarMethod](char c) {
                                switch (c) {
                                    case '^': case '(':
                                    case '\'': case '`': case '?': case '~': case '.':
                                    case '\\':
                                        return true;
                                    case '+': return isViqrMethod;
                                    case '*': return isViqrStarMethod;
                                    default:  return false;
                                }
                            }(viqrKey);

    if (!viqrBypass && event.aux == KeyAux::None && event.key != '\0' &&
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
    if ((config_.method == farolkey::core::InputMethod::Telex       && key == 'z')  ||
        (config_.method == farolkey::core::InputMethod::SimpleTelex2 && key == 'z')  ||
        (config_.method == farolkey::core::InputMethod::Vni          && key == '0')  ||
        (config_.method == farolkey::core::InputMethod::TocKy        && key == '0')  ||
        (config_.method == farolkey::core::InputMethod::Viqr         && key == '\\') ||
        (config_.method == farolkey::core::InputMethod::ViqrStar     && key == '\\')) {
        clearRepeatTransformState();
        const bool removed =
            (config_.method == farolkey::core::InputMethod::Vni ||
             config_.method == farolkey::core::InputMethod::TocKy)
                ? vi_syllable::removeVniDiacritics(decoded)
                : vi_syllable::removeTelexDiacritics(decoded);
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
        ((config_.method == farolkey::core::InputMethod::Telex       && isTelexRepeatableKey(key))       ||
         (config_.method == farolkey::core::InputMethod::Vni         && isVniRepeatableKey(key))         ||
         (config_.method == farolkey::core::InputMethod::Viqr        && isViqrRepeatableKey(key))        ||
         (config_.method == farolkey::core::InputMethod::ViqrStar    && isViqrStarRepeatableKey(key))    ||
         (config_.method == farolkey::core::InputMethod::SimpleTelex  && isSimpleTelexRepeatableKey(key)) ||
         (config_.method == farolkey::core::InputMethod::SimpleTelex2 && isTelexRepeatableKey(key))        ||
         (config_.method == farolkey::core::InputMethod::Microsoft    && isMicrosoftRepeatableKey(key))    ||
         (config_.method == farolkey::core::InputMethod::TocKy        && isTocKyRepeatableKey(key)))) {
        std::u32string reverted = decodeUtf8Normalized(repeatTransformState_.buffer_before);
        reverted.push_back(static_cast<unsigned char>(raw));
        if (config_.method == farolkey::core::InputMethod::Telex) {
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
    if (config_.method == farolkey::core::InputMethod::Telex) {
        if (key == 's' || key == 'f' || key == 'r' || key == 'x' || key == 'j') {
            transformed = applyTone(decoded, key);
        } else {
            transformed = vi_syllable::applyTelexTransform(decoded, key);
        }
    } else if (config_.method == farolkey::core::InputMethod::Vni) {
        if (key >= '1' && key <= '5') {
            transformed = applyToneVni(decoded, key);
        } else {
            transformed = vi_syllable::applyVniTransform(decoded, key);
        }
    } else if (config_.method == farolkey::core::InputMethod::Viqr) {
        if (key == '\'' || key == '`' || key == '?' || key == '~' || key == '.') {
            transformed = applyToneViqr(decoded, key);
        } else {
            transformed = vi_syllable::applyViqrTransform(decoded, key);
        }
    } else if (config_.method == farolkey::core::InputMethod::ViqrStar) {
        if (key == '\'' || key == '`' || key == '?' || key == '~' || key == '.') {
            transformed = applyToneViqr(decoded, key);
        } else {
            transformed = vi_syllable::applyViqrStarTransform(decoded, key);
        }
    } else if (config_.method == farolkey::core::InputMethod::SimpleTelex) {
        // Only the 5 tone keys are active; all other keys are literal.
        if (key == 's' || key == 'f' || key == 'r' || key == 'x' || key == 'j') {
            transformed = applyTone(decoded, key);
        }
    } else if (config_.method == farolkey::core::InputMethod::SimpleTelex2) {
        if (key == 's' || key == 'f' || key == 'r' || key == 'x' || key == 'j') {
            transformed = applyTone(decoded, key);
        } else {
            transformed = vi_syllable::applyTelexTransform(decoded, key);
            // vne_telex_w fallback: 'w' with no hookable vowel → standalone ư.
            // This is the sole difference from regular Telex.
            if (!transformed && key == 'w') {
                decoded.push_back(U'ư');
                transformed = true;
            }
        }
    } else if (config_.method == farolkey::core::InputMethod::Microsoft) {
        // Number keys directly insert Vietnamese characters; no base letter needed.
        // Tone keys 5/6/9 map to Telex equivalents f/r/x; s and j keep their Telex role.
        char32_t directChar = 0;
        switch (key) {
            case '1': directChar = U'ă'; break;
            case '2': directChar = U'â'; break;
            case '3': directChar = U'ê'; break;
            case '4': directChar = U'ô'; break;
            case '7': directChar = U'ư'; break;
            case '8': directChar = U'ơ'; break;
            case '0': directChar = U'đ'; break;
            default:  break;
        }
        if (directChar) {
            decoded.push_back(directChar);
            transformed = true;
        } else {
            char toneKey = 0;
            if      (key == 's') toneKey = 's';
            else if (key == 'j') toneKey = 'j';
            else if (key == '5') toneKey = 'f';  // huyền
            else if (key == '6') toneKey = 'r';  // hỏi
            else if (key == '9') toneKey = 'x';  // ngã
            if (toneKey) transformed = applyTone(decoded, toneKey);
        }
    } else if (config_.method == farolkey::core::InputMethod::TocKy) {
        // Base: VNI tones (1-5) and diacritic transforms (0, 6-9).
        if (key >= '1' && key <= '5') {
            transformed = applyToneVni(decoded, key);
        } else if (key == '0' || (key >= '6' && key <= '9')) {
            transformed = vi_syllable::applyVniTransform(decoded, key);
        }
        // Consonant shortcuts (only when VNI base did not handle the key).
        if (!transformed) {
            const bool hasVowel = hasVietnameseVowel(decoded);
            if (!hasVowel) {
                // Initial shortcuts: active when no vowel in buffer yet.
                // Note: 'k' is NOT a shortcut here — it stays literal so users can
                // type words spelled with 'k' (e.g. "kế", "kỳ") without confusion.
                // The sound /kh/ is typed literally as 'k'+'h'.
                switch (key) {
                    case 'f': decoded.push_back(U'p'); decoded.push_back(U'h'); transformed = true; break;
                    case 'j': decoded.push_back(U'g'); decoded.push_back(U'i'); transformed = true; break;
                    case 'z': decoded.push_back(U'd');                          transformed = true; break;
                    case 'd': decoded.push_back(U'đ');                     transformed = true; break;  // đ
                    case 'w': decoded.push_back(U'n'); decoded.push_back(U'g'); transformed = true; break;
                    case 'q': decoded.push_back(U'q'); decoded.push_back(U'u'); transformed = true; break;
                    default:  break;
                }
            } else {
                // Final shortcuts: active after a vowel is present.
                switch (key) {
                    case 'g': decoded.push_back(U'n'); decoded.push_back(U'g'); transformed = true; break;
                    case 'h': decoded.push_back(U'n'); decoded.push_back(U'h'); transformed = true; break;
                    case 'k': decoded.push_back(U'c'); decoded.push_back(U'h'); transformed = true; break;
                    default:  break;
                }
            }
        }
    }

    if (transformed) {
        // After any transform, normalize so the preedit reflects the final shape.
        // Telex: converts intermediate "đuợ" → "đượ" (u→ư when nucleus starts with uơ).
        // VNI:   converts 'uo'→'uơ' after '7' transform (only when coda is present).
        if (config_.method == farolkey::core::InputMethod::Telex ||
            config_.method == farolkey::core::InputMethod::SimpleTelex2) {
            vi_syllable::normalizeTelexBuffer(decoded);
        } else if (config_.method == farolkey::core::InputMethod::Vni ||
                   config_.method == farolkey::core::InputMethod::TocKy) {
            vi_syllable::normalizeVniUoTransform(decoded);
        }
        preeditBuffer_ = encodeUtf8(decoded);
        if ((config_.method == farolkey::core::InputMethod::Telex        && isTelexRepeatableKey(key))        ||
            (config_.method == farolkey::core::InputMethod::Vni          && isVniRepeatableKey(key))          ||
            (config_.method == farolkey::core::InputMethod::Viqr         && isViqrRepeatableKey(key))         ||
            (config_.method == farolkey::core::InputMethod::ViqrStar     && isViqrStarRepeatableKey(key))     ||
            (config_.method == farolkey::core::InputMethod::SimpleTelex  && isSimpleTelexRepeatableKey(key))  ||
            (config_.method == farolkey::core::InputMethod::SimpleTelex2 && isTelexRepeatableKey(key))        ||
            (config_.method == farolkey::core::InputMethod::Microsoft    && isMicrosoftRepeatableKey(key))    ||
            (config_.method == farolkey::core::InputMethod::TocKy        && isTocKyRepeatableKey(key))) {
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
    if (config_.method == farolkey::core::InputMethod::Telex ||
        config_.method == farolkey::core::InputMethod::SimpleTelex2) {
        vi_syllable::normalizeTelexBuffer(decoded);
    } else if (config_.method == farolkey::core::InputMethod::Vni ||
               config_.method == farolkey::core::InputMethod::TocKy) {
        // fromPushPath=true: also normalizes "uơi"→"ươi" (e.g. "người" from "nguo7i").
        vi_syllable::normalizeVniUoTransform(decoded, /*fromPushPath=*/true);
    }
    preeditBuffer_ = encodeUtf8(decoded);
    applyPreeditNormalize(decoded);
    if (const auto split = maybeAutoCommitStablePrefix(decoded)) {
        return *split;
    }
    return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
}

ProcessResult Engine::processEnglishKey(const KeyEvent& event) {
    clearRepeatTransformState();
    clearPendingLiteralEscape();

    // EN mode uses immediate-commit: each character is committed to the app
    // immediately (no preedit), so it appears in terminal without waiting for space.
    // preeditBuffer_ is kept as an internal word-tracking buffer only.
    // On word boundary, if the tracked word matches a dict/template entry,
    // deleteSurroundingBefore instructs the adapter to delete the already-committed
    // characters before inserting the expansion.

    auto flushEnBuffer = [this](std::string suffix) -> ProcessResult {
        ProcessResult r;
        if (!preeditBuffer_.empty()) {
            if (config_.enableUserDictionary && !userDict_.empty() && !passwordField_) {
                if (const UserDictEntry* e = userDict_.lookup(preeditBuffer_)) {
                    if (e->abbrev_mode == AbbrevMode::En || e->abbrev_mode == AbbrevMode::Both) {
                        r.deleteSurroundingBefore = static_cast<int>(preeditBuffer_.size());
                        r.commit = e->expansion + suffix;
                        preeditBuffer_.clear();
                        r.consumed = true;
                        return r;
                    }
                }
            }
            // M15: Smart Templates — try parametric expansion after exact-match fails.
            if (config_.enableSmartTemplates && !passwordField_) {
                const std::string expanded = tryParametricExpand(preeditBuffer_, "en");
                if (!expanded.empty()) {
                    r.deleteSurroundingBefore = static_cast<int>(preeditBuffer_.size());
                    r.commit = expanded;
                    preeditBuffer_.clear();
                    r.consumed = true;
                    r.smartTemplateExpansion = true;
                    return r;
                }
            }
            // No match: chars already committed individually — just commit suffix.
            preeditBuffer_.clear();
            if (!suffix.empty()) r.commit = suffix;
        } else if (!suffix.empty()) {
            r.commit = suffix;
        }
        r.consumed = true;
        return r;
    };

    if (event.aux != KeyAux::None) {
        // Non-printable key (Tab, Enter, arrow, etc.): flush tracker, forward key.
        if (!preeditBuffer_.empty()) {
            auto r = flushEnBuffer("");
            r.forwardOriginalKey = true;
            return r;
        }
        return ProcessResult{};  // nothing pending → forward key normally
    }

    if (event.key == '\0') {
        return ProcessResult{};
    }

    const auto uc = static_cast<unsigned char>(event.key);
    if (uc < 0x20 || event.key == '\x7F') {
        // Backspace: remove from internal tracker, let the app handle the visual deletion.
        if (event.key == '\b' && !preeditBuffer_.empty()) {
            preeditBuffer_.pop_back();
        }
        return ProcessResult{};
    }

    // Space (and any non-letter word boundary) flushes the tracker.
    if (event.key == ' ') {
        return flushEnBuffer(" ");
    }

    // Printable non-space: commit immediately so it appears in the app (terminal)
    // without waiting for a word boundary. Also add to tracker for dict lookup.
    preeditBuffer_ += event.key;
    ProcessResult r;
    r.commit   = std::string(1, event.key);
    r.consumed = true;
    return r;
}

void Engine::seedPreeditForCommittedRewrite(std::string utf8) {
    clearState();
    preeditBuffer_ = std::move(utf8);
}

std::optional<std::string> Engine::tryRewriteCommittedSyllable(const farolkey::config::RuntimeConfig& config,
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
    if (config.method == farolkey::core::InputMethod::Vni && event.key >= '1' && event.key <= '5') {
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

}  // namespace farolkey::core
