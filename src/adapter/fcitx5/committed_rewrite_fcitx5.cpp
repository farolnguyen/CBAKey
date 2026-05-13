#include "cbakey/adapter/fcitx5/committed_rewrite_fcitx5.h"

#include <cctype>
#include <optional>
#include <string>
#include <vector>

#include <fcitx/inputcontext.h>
#include <fcitx/surroundingtext.h>
#include <fcitx-utils/capabilityflags.h>

#include "cbakey/adapter/fcitx5/surrounding_cursor_normalize.h"
#include "cbakey/core/engine.h"

namespace cbakey::adapter::fcitx5 {
namespace {

bool decodeOneUtf8(const std::string& s, std::size_t* i, char32_t* out) {
    if (*i >= s.size()) {
        return false;
    }
    const unsigned char c0 = static_cast<unsigned char>(s[*i]);
    if ((c0 & 0x80U) == 0U) {
        *out = static_cast<char32_t>(c0);
        ++(*i);
        return true;
    }
    if (*i + 1 < s.size() && (c0 & 0xE0U) == 0xC0U) {
        const char32_t cp =
            ((static_cast<char32_t>(c0) & 0x1FU) << 6) |
            (static_cast<unsigned char>(s[*i + 1]) & 0x3FU);
        *out = cp;
        *i += 2;
        return true;
    }
    if (*i + 2 < s.size() && (c0 & 0xF0U) == 0xE0U) {
        const char32_t cp =
            ((static_cast<char32_t>(c0) & 0x0FU) << 12) |
            ((static_cast<unsigned char>(s[*i + 1]) & 0x3FU) << 6) |
            (static_cast<unsigned char>(s[*i + 2]) & 0x3FU);
        *out = cp;
        *i += 3;
        return true;
    }
    if (*i + 3 < s.size() && (c0 & 0xF8U) == 0xF0U) {
        const char32_t cp =
            ((static_cast<char32_t>(c0) & 0x07U) << 18) |
            ((static_cast<unsigned char>(s[*i + 1]) & 0x3FU) << 12) |
            ((static_cast<unsigned char>(s[*i + 2]) & 0x3FU) << 6) |
            (static_cast<unsigned char>(s[*i + 3]) & 0x3FU);
        *out = cp;
        *i += 4;
        return true;
    }
    ++(*i);
    *out = U'?';
    return true;
}

bool isBoundaryChar(char32_t cp) {
    if (cp <= 32) {
        return true;
    }
    if (cp < 128) {
        switch (static_cast<char>(cp)) {
            case ',':
            case '.':
            case ';':
            case ':':
            case '!':
            case '?':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '"':
            case '\'':
            case '/':
            case '\\':
            case '|':
            case '@':
            case '#':
            case '$':
            case '%':
            case '^':
            case '&':
            case '*':
            case '+':
            case '=':
            case '<':
            case '>':
            case '~':
            case '`':
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool isWordChar(char32_t cp) {
    if (isBoundaryChar(cp)) {
        return false;
    }
    if (cp <= 127) {
        return std::isalnum(static_cast<unsigned char>(cp)) != 0 || cp == U'_';
    }
    if ((cp >= 0x00C0 && cp <= 0x024F) || (cp >= 0x1E00 && cp <= 0x1EFF) || (cp >= 0x2C60 && cp <= 0x2C7F)) {
        return true;
    }
    return true;
}

struct Utf8Cp {
    std::size_t byte_begin = 0;
    char32_t ch = 0;
};

std::vector<Utf8Cp> indexUtf8(const std::string& s) {
    std::vector<Utf8Cp> out;
    std::size_t i = 0;
    while (i < s.size()) {
        const std::size_t b = i;
        char32_t ch = 0;
        if (!decodeOneUtf8(s, &i, &ch)) {
            break;
        }
        out.push_back(Utf8Cp{.byte_begin = b, .ch = ch});
    }
    return out;
}

/// Returns UTF-8 token immediately left of caret (caret = UCS-4 index into \p text) and its
/// length in UCS-4 codepoints, or nullopt if caret is not flush after a word token.
std::optional<std::pair<std::string, int>> tokenLeftOfCaret(const std::string& text,
                                                             unsigned int cursor_chars) {
    const std::vector<Utf8Cp> cps = indexUtf8(text);
    if (cursor_chars > cps.size()) {
        return std::nullopt;
    }
    if (cursor_chars == 0) {
        return std::nullopt;
    }
    const std::size_t lastIdx = static_cast<std::size_t>(cursor_chars) - 1;
    if (!isWordChar(cps[lastIdx].ch)) {
        return std::nullopt;
    }
    std::size_t startIdx = lastIdx;
    while (startIdx > 0 && isWordChar(cps[startIdx - 1].ch)) {
        --startIdx;
    }
    const std::size_t byteStart = cps[startIdx].byte_begin;
    const std::size_t byteEnd =
        lastIdx + 1 < cps.size() ? cps[lastIdx + 1].byte_begin : text.size();
    const std::string token = text.substr(byteStart, byteEnd - byteStart);
    const int token_chars = static_cast<int>(lastIdx - startIdx + 1);
    return std::make_pair(token, token_chars);
}

bool isCommittedRewriteTrigger(const cbakey::config::RuntimeConfig& config,
                               const cbakey::core::KeyEvent& ev) {
    if (ev.ctrl || ev.alt) {
        return false;
    }
    if (ev.aux != cbakey::core::KeyAux::None) {
        return false;
    }
    if (ev.key_from_keypad) {
        return false;
    }
    if (ev.key == '\0' || ev.key == '\b' || ev.key == ' ' || ev.key == '\n' || ev.key == '\t') {
        return false;
    }
    const unsigned char uk = static_cast<unsigned char>(ev.key);
    if (config.method == cbakey::core::InputMethod::Telex) {
        const char k = static_cast<char>(std::tolower(uk));
        switch (k) {
            case 's':
            case 'f':
            case 'r':
            case 'x':
            case 'j':
            case 'z':
            case 'w':
                return true;
            default:
                return false;
        }
    }
    if (std::isdigit(uk) != 0) {
        return true;
    }
    return false;
}

}  // namespace

bool tryApplyCommittedSyllableRewrite(fcitx::InputContext* ic,
                                      const cbakey::config::RuntimeConfig& config,
                                      const cbakey::core::KeyEvent& event) {
    if (!ic || !config.fcitx5CommittedRewrite || !isCommittedRewriteTrigger(config, event)) {
        return false;
    }
    if (!ic->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText)) {
        return false;
    }
    // Ask fcitx to emit SurroundingTextUpdatedEvent so flaky clients (e.g. Electron/Teams) may
    // refresh cached surrounding before we read it.
    ic->updateSurroundingText();
    const auto& st = ic->surroundingText();
    if (!st.isValid() || !st.selectedText().empty()) {
        return false;
    }
    const unsigned int cursor_chars =
        normalizeSurroundingCursorToCodepointIndex(st.text(), st.cursor());
    const auto tokenInfo = tokenLeftOfCaret(st.text(), cursor_chars);
    if (!tokenInfo) {
        return false;
    }
    const std::string& token = tokenInfo->first;
    const int tokenChars = tokenInfo->second;
    if (tokenChars <= 0) {
        return false;
    }
    const auto rewritten =
        cbakey::core::Engine::tryRewriteCommittedSyllable(config, token, event);
    if (!rewritten) {
        return false;
    }
    ic->deleteSurroundingText(-tokenChars, static_cast<unsigned int>(tokenChars));
    ic->commitString(*rewritten);
    return true;
}

}  // namespace cbakey::adapter::fcitx5
