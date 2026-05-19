#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "farolkey/config/config.h"
#include "farolkey/core/engine.h"
#include "farolkey/core/vi_syllable.h"

namespace fs = std::filesystem;

namespace {

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::core::ProcessResult;

struct AuditStats {
    int tone_supported = 0;
    int tone_fallback = 0;
    int transform_supported = 0;
    int transform_fallback = 0;
    int literal_supported = 0;
    std::map<std::string, int> fallback_buckets;
    std::vector<std::string> fallback_samples;
};

std::u32string decodeUtf8(const std::string& input) {
    std::u32string output;
    for (std::size_t i = 0; i < input.size();) {
        const unsigned char c = static_cast<unsigned char>(input[i]);
        if ((c & 0x80U) == 0U) {
            output.push_back(static_cast<char32_t>(c));
            ++i;
        } else if ((c & 0xE0U) == 0xC0U && i + 1 < input.size()) {
            const char32_t cp = ((c & 0x1FU) << 6U) |
                                (static_cast<unsigned char>(input[i + 1]) & 0x3FU);
            output.push_back(cp);
            i += 2;
        } else if ((c & 0xF0U) == 0xE0U && i + 2 < input.size()) {
            const char32_t cp = ((c & 0x0FU) << 12U) |
                                ((static_cast<unsigned char>(input[i + 1]) & 0x3FU) << 6U) |
                                (static_cast<unsigned char>(input[i + 2]) & 0x3FU);
            output.push_back(cp);
            i += 3;
        } else if ((c & 0xF8U) == 0xF0U && i + 3 < input.size()) {
            const char32_t cp = ((c & 0x07U) << 18U) |
                                ((static_cast<unsigned char>(input[i + 1]) & 0x3FU) << 12U) |
                                ((static_cast<unsigned char>(input[i + 2]) & 0x3FU) << 6U) |
                                (static_cast<unsigned char>(input[i + 3]) & 0x3FU);
            output.push_back(cp);
            i += 4;
        } else {
            ++i;
        }
    }
    return output;
}

bool parseStep(const nlohmann::json& step, KeyEvent* ev) {
    if (!step.is_object()) {
        return false;
    }
    *ev = KeyEvent{};
    if (step.contains("key")) {
        const std::string key = step["key"].get<std::string>();
        if (key.size() != 1U) {
            return false;
        }
        ev->key = key[0];
    }
    ev->ctrl = step.value("ctrl", false);
    ev->alt = step.value("alt", false);
    ev->shift = step.value("shift", false);
    return true;
}

bool isTelexTone(char key) {
    return key == 's' || key == 'f' || key == 'r' || key == 'x' || key == 'j';
}

bool isTelexTransform(char key) {
    return key == 'a' || key == 'e' || key == 'o' || key == 'w' || key == 'd';
}

bool isVniTone(char key) {
    return key >= '1' && key <= '5';
}

bool isVniTransform(char key) {
    return key >= '6' && key <= '9';
}

bool hasTag(const nlohmann::json& obj, const std::string& want) {
    if (!obj.contains("tags") || !obj["tags"].is_array()) {
        return false;
    }
    for (const auto& tag : obj["tags"]) {
        if (tag.is_string() && tag.get<std::string>() == want) {
            return true;
        }
    }
    return false;
}

void auditCase(const nlohmann::json& obj, AuditStats* stats) {
    farolkey::config::RuntimeConfig cfg = farolkey::config::defaultConfig();
    const std::string config = obj.value("config", "default");
    if (config == "vni") {
        cfg.method = InputMethod::Vni;
    }

    if (!obj.contains("sequence") || !obj["sequence"].is_array()) {
        return;
    }

    Engine engine(cfg);
    std::string currentPreedit;
    const std::string caseId = obj.value("id", std::string{"<no-id>"});
    for (const auto& step : obj["sequence"]) {
        KeyEvent ev{};
        if (!parseStep(step, &ev)) {
            continue;
        }

        const std::string beforePreedit = currentPreedit;
        const char rawKey = ev.key;
        const char key = static_cast<char>(std::tolower(static_cast<unsigned char>(rawKey)));
        const ProcessResult result = engine.processKey(ev);
        const std::string appended = beforePreedit + std::string(1, rawKey);
        const bool changedPreedit = result.commit.empty() && result.preedit != appended;

        if (changedPreedit) {
            const std::u32string decoded = decodeUtf8(beforePreedit);
            const bool telexRepeatEscape = cfg.method == InputMethod::Telex && hasTag(obj, "telex.repeat_escape") &&
                                           (isTelexTone(key) || isTelexTransform(key));
            const bool telexRawEscape = cfg.method == InputMethod::Telex && hasTag(obj, "telex.raw_escape");
            const bool vniRepeatEscape = cfg.method == InputMethod::Vni && hasTag(obj, "vni.repeat_escape") &&
                                         isVniTone(key);
            const bool vniRawEscape = cfg.method == InputMethod::Vni && hasTag(obj, "vni.raw_escape");
            const bool vniRemoveDiacritics = cfg.method == InputMethod::Vni && hasTag(obj, "vni.remove_diacritics");
            if (cfg.method == InputMethod::Telex) {
                if (isTelexTone(key)) {
                    if (farolkey::core::vi_syllable::selectToneVowelIndex(decoded)) {
                        ++stats->tone_supported;
                    } else if (telexRepeatEscape) {
                        ++stats->literal_supported;
                    } else {
                        ++stats->tone_fallback;
                        ++stats->fallback_buckets[std::string("telex.tone.") + key];
                    }
                } else if (isTelexTransform(key)) {
                    std::u32string copy = decoded;
                    if (farolkey::core::vi_syllable::applyTelexTransform(copy, key)) {
                        ++stats->transform_supported;
                    } else if (telexRepeatEscape) {
                        ++stats->literal_supported;
                    } else {
                        ++stats->transform_fallback;
                        ++stats->fallback_buckets[std::string("telex.transform.") + key];
                    }
                } else if (key == 'z') {
                    std::u32string copy = decoded;
                    if (farolkey::core::vi_syllable::removeTelexDiacritics(copy)) {
                        ++stats->literal_supported;
                    }
                } else if (telexRawEscape) {
                    ++stats->literal_supported;
                } else {
                    std::u32string copy = decodeUtf8(appended);
                    if (farolkey::core::vi_syllable::normalizeTelexBuffer(copy)) {
                        ++stats->literal_supported;
                    }
                }
            } else {
                if (isVniTone(key)) {
                    if (farolkey::core::vi_syllable::selectToneVowelIndex(decoded)) {
                        ++stats->tone_supported;
                    } else if (vniRepeatEscape) {
                        ++stats->literal_supported;
                    } else {
                        ++stats->tone_fallback;
                        ++stats->fallback_buckets[std::string("vni.tone.") + key];
                    }
                } else if (isVniTransform(key)) {
                    std::u32string copy = decoded;
                    if (farolkey::core::vi_syllable::applyVniTransform(copy, key)) {
                        ++stats->transform_supported;
                    } else {
                        ++stats->transform_fallback;
                        ++stats->fallback_buckets[std::string("vni.transform.") + key];
                    }
                } else if (key == '0' && vniRemoveDiacritics) {
                    std::u32string copy = decoded;
                    if (farolkey::core::vi_syllable::removeVniDiacritics(copy)) {
                        ++stats->literal_supported;
                    }
                } else if (key == '\\' && vniRawEscape) {
                    ++stats->literal_supported;
                }
            }

            if (stats->fallback_samples.size() < 20U) {
                const std::u32string decoded = decodeUtf8(beforePreedit);
                bool handledByNew = false;
                if (cfg.method == InputMethod::Telex) {
                    if (isTelexTone(key)) {
                        handledByNew = farolkey::core::vi_syllable::selectToneVowelIndex(decoded).has_value();
                    } else if (isTelexTransform(key)) {
                        auto copy = decoded;
                        handledByNew = farolkey::core::vi_syllable::applyTelexTransform(copy, key);
                    } else if (key == 'z') {
                        auto copy = decoded;
                        handledByNew = farolkey::core::vi_syllable::removeTelexDiacritics(copy);
                    } else {
                        auto copy = decodeUtf8(appended);
                        handledByNew = farolkey::core::vi_syllable::normalizeTelexBuffer(copy);
                    }
                    if (!handledByNew && (telexRepeatEscape || telexRawEscape)) {
                        handledByNew = true;
                    }
                } else {
                    if (isVniTone(key)) {
                        handledByNew = farolkey::core::vi_syllable::selectToneVowelIndex(decoded).has_value();
                        if (!handledByNew && vniRepeatEscape) {
                            handledByNew = true;
                        }
                    } else if (isVniTransform(key)) {
                        auto copy = decoded;
                        handledByNew = farolkey::core::vi_syllable::applyVniTransform(copy, key);
                    } else if (key == '0' && vniRemoveDiacritics) {
                        auto copy = decoded;
                        handledByNew = farolkey::core::vi_syllable::removeVniDiacritics(copy);
                    } else if (key == '\\' && vniRawEscape) {
                        handledByNew = true;
                    }
                }
                if (!handledByNew) {
                    stats->fallback_samples.push_back(caseId + " before=\"" + beforePreedit + "\" key=\"" +
                                                      std::string(1, rawKey) + "\" after=\"" + result.preedit + "\"");
                }
            }
        }

        currentPreedit = result.preedit;
    }
}

void auditJsonl(const fs::path& path, AuditStats* stats) {
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        const auto obj = nlohmann::json::parse(line);
        if (obj.value("skip", false)) {
            continue;
        }
        auditCase(obj, stats);
    }
}

}  // namespace

int main() {
    const fs::path root = fs::exists("corpus/final") ? fs::path("corpus/final") : fs::path("corpus");
    AuditStats stats;
    for (const fs::path& path : {root / "telex.jsonl", root / "vni.jsonl"}) {
        if (fs::exists(path)) {
            auditJsonl(path, &stats);
        }
    }

    std::cout << "{\n"
              << "  \"corpus_root\": \"" << root.string() << "\",\n"
              << "  \"tone_supported\": " << stats.tone_supported << ",\n"
              << "  \"tone_fallback\": " << stats.tone_fallback << ",\n"
              << "  \"transform_supported\": " << stats.transform_supported << ",\n"
              << "  \"transform_fallback\": " << stats.transform_fallback << ",\n"
              << "  \"literal_supported\": " << stats.literal_supported << ",\n"
              << "  \"fallback_buckets\": {\n";
    bool first = true;
    for (const auto& [key, count] : stats.fallback_buckets) {
        if (!first) {
            std::cout << ",\n";
        }
        first = false;
        std::cout << "    \"" << key << "\": " << count;
    }
    std::cout << "\n  },\n"
              << "  \"fallback_samples\": [\n";
    for (std::size_t i = 0; i < stats.fallback_samples.size(); ++i) {
        std::cout << "    " << nlohmann::json(stats.fallback_samples[i]).dump();
        if (i + 1 < stats.fallback_samples.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "  ]\n"
              << "}\n";
    return 0;
}
