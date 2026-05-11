#include "cbakey/core/engine.h"

#include <array>
#include <cctype>
#include <optional>
#include <vector>

#include "cbakey/core/vi_syllable.h"

namespace cbakey::core {

namespace {

constexpr std::size_t kToneCount = 6;  // ngang, sac, huyen, hoi, nga, nang

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

std::string encodeUtf8(const std::u32string& input) {
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
    const auto& sets = toneSets();
    for (const auto& set : sets) {
        for (std::size_t k = 0; k < kToneCount; ++k) {
            if (set[k] == ch) {
                ch = set[toneIndex];
                return true;
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

}  // namespace

Engine::Engine(cbakey::config::RuntimeConfig config) : config_(std::move(config)) {}

bool Engine::isAsciiSeparatorCommit(char ch) {
    const auto u = static_cast<unsigned char>(ch);
    return u < 128 && std::ispunct(u) != 0;
}

std::string Engine::takeCompositionForCommit() {
    std::string out = std::move(preeditBuffer_);
    preeditHistory_.clear();
    return out;
}

ProcessResult Engine::processKey(const KeyEvent& event) {
    if (isToggleHotkey(event)) {
        mode_ = (mode_ == InputMode::English) ? InputMode::Vietnamese : InputMode::English;
        return ProcessResult{.preedit = "", .commit = "", .consumed = true};
    }

    return mode_ == InputMode::Vietnamese ? processVietnameseKey(event) : processEnglishKey(event);
}

void Engine::setInputMode(InputMode mode) {
    mode_ = mode;
}

InputMode Engine::inputMode() const {
    return mode_;
}

void Engine::clearState() {
    preeditBuffer_.clear();
    preeditHistory_.clear();
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
        r.commit = preeditBuffer_ + std::move(suffix);
        preeditBuffer_.clear();
        preeditHistory_.clear();
        r.consumed = true;
        return r;
    };

    switch (event.aux) {
        case KeyAux::None:
            break;
        case KeyAux::Left:
        case KeyAux::Right:
        case KeyAux::Home:
        case KeyAux::End:
            if (!preeditBuffer_.empty()) {
                ProcessResult r;
                r.commit = preeditBuffer_;
                preeditBuffer_.clear();
                preeditHistory_.clear();
                r.consumed = true;
                r.forwardOriginalKey = true;
                return r;
            }
            return ProcessResult{};
        case KeyAux::DeleteForward:
            if (!preeditBuffer_.empty()) {
                if (!preeditHistory_.empty()) {
                    preeditBuffer_ = preeditHistory_.back();
                    preeditHistory_.pop_back();
                    return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
                }
                std::u32string decoded = decodeUtf8(preeditBuffer_);
                if (!decoded.empty()) {
                    decoded.pop_back();
                    preeditBuffer_ = encodeUtf8(decoded);
                    return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
                }
            }
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
        if (!preeditHistory_.empty()) {
            preeditBuffer_ = preeditHistory_.back();
            preeditHistory_.pop_back();
            return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
        }

        std::u32string decoded = decodeUtf8(preeditBuffer_);
        if (!decoded.empty()) {
            decoded.pop_back();
            preeditBuffer_ = encodeUtf8(decoded);
            return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
        }

        return ProcessResult{.preedit = "", .commit = "", .consumed = false};
    }

    if (event.key == ' ') {
        return commitWithSuffix(" ");
    }

    if (event.aux == KeyAux::None && event.key != '\0' &&
        isAsciiSeparatorCommit(static_cast<char>(event.key))) {
        if (!preeditBuffer_.empty()) {
            return commitWithSuffix(std::string(1, static_cast<char>(event.key)));
        }
        return ProcessResult{};
    }

    const char raw = event.key;
    const char key = static_cast<char>(std::tolower(static_cast<unsigned char>(raw)));
    preeditHistory_.push_back(preeditBuffer_);

    std::u32string decoded = decodeUtf8(preeditBuffer_);
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
        return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
    }

    decoded.push_back(static_cast<unsigned char>(raw));
    if (config_.method == cbakey::core::InputMethod::Telex) {
        vi_syllable::normalizeTelexBuffer(decoded);
    }
    preeditBuffer_ = encodeUtf8(decoded);
    return ProcessResult{.preedit = preeditBuffer_, .commit = "", .consumed = true};
}

ProcessResult Engine::processEnglishKey(const KeyEvent& event) {
    if (event.aux != KeyAux::None) {
        return ProcessResult{};
    }
    if (event.key == '\0') {
        return ProcessResult{.preedit = "", .commit = "", .consumed = false};
    }

    return ProcessResult{.preedit = "", .commit = std::string(1, event.key), .consumed = true};
}

}  // namespace cbakey::core
