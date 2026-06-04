#include "corpus_driver.h"

#include <sstream>
#include <unordered_map>

#include "farolkey/config/config.h"
#include "farolkey/core/engine.h"
#include "farolkey/core/free_layout_config.h"

namespace farolkey::test {
namespace {

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyAux;
using farolkey::core::KeyEvent;
using farolkey::core::ProcessResult;

bool auxFromString(const std::string& s, KeyAux* out, std::string* err) {
    static const std::unordered_map<std::string, KeyAux> kMap = {
        {"None", KeyAux::None},
        {"Enter", KeyAux::Enter},
        {"Tab", KeyAux::Tab},
        {"Left", KeyAux::Left},
        {"Right", KeyAux::Right},
        {"Up", KeyAux::Up},
        {"Down", KeyAux::Down},
        {"Home", KeyAux::Home},
        {"End", KeyAux::End},
        {"DeleteForward", KeyAux::DeleteForward},
    };
    const auto it = kMap.find(s);
    if (it == kMap.end()) {
        *err = "unknown aux: " + s;
        return false;
    }
    *out = it->second;
    return true;
}

char asciiKeyFromString(const std::string& k, std::string* err) {
    if (k.empty()) {
        *err = "empty key string";
        return '\0';
    }
    const unsigned char c0 = static_cast<unsigned char>(k[0]);
    if (c0 >= 0x80U) {
        *err = "sequence key must be ASCII (schema v1)";
        return '\0';
    }
    if (k.size() != 1U) {
        *err = "sequence key must be exactly one byte (schema v1)";
        return '\0';
    }
    return static_cast<char>(k[0]);
}

bool parseKeyStep(const nlohmann::json& step, KeyEvent* ev, std::string* err) {
    err->clear();
    *ev = KeyEvent{};
    if (!step.is_object()) {
        *err = "sequence step must be object";
        return false;
    }
    if (step.contains("aux") && !step["aux"].is_null()) {
        const std::string an = step["aux"].get<std::string>();
        if (!auxFromString(an, &ev->aux, err)) {
            return false;
        }
    }
    if (step.contains("key")) {
        const std::string ks = step["key"].get<std::string>();
        ev->key = asciiKeyFromString(ks, err);
        if (!err->empty()) {
            return false;
        }
    }
    ev->ctrl = step.value("ctrl", false);
    ev->alt = step.value("alt", false);
    ev->shift = step.value("shift", false);
    ev->key_from_keypad = step.value("key_from_keypad", false);
    if (ev->aux == KeyAux::None && ev->key == '\0') {
        *err = "sequence step needs key or aux";
        return false;
    }
    return true;
}

void appendMetaDiff(std::ostringstream& oss,
                      const ProcessResult& got,
                      bool wantConsumed,
                      bool wantForward) {
    if (got.consumed != wantConsumed) {
        oss << " consumed want " << wantConsumed << " got " << got.consumed;
    }
    if (got.forwardOriginalKey != wantForward) {
        oss << " forwardOriginalKey want " << wantForward << " got " << got.forwardOriginalKey;
    }
}

bool checkExpectBlock(const std::string& label,
                      const ProcessResult& got,
                      const nlohmann::json& exp,
                      bool requireMeta,
                      std::string* err) {
    if (!exp.contains("preedit") || !exp.contains("commit")) {
        *err = label + ": expect block needs preedit and commit";
        return false;
    }
    const std::string wantPre = exp["preedit"].get<std::string>();
    const std::string wantCom = exp["commit"].get<std::string>();
    if (got.preedit != wantPre) {
        *err = label + " preedit want \"" + wantPre + "\" got \"" + got.preedit + "\"";
        return false;
    }
    if (got.commit != wantCom) {
        *err = label + " commit want \"" + wantCom + "\" got \"" + got.commit + "\"";
        return false;
    }
    if (requireMeta) {
        if (!exp.contains("consumed") || !exp.contains("forward_original_key")) {
            *err = label + ": assert_meta requires consumed and forward_original_key";
            return false;
        }
        const bool wc = exp["consumed"].get<bool>();
        const bool wf = exp["forward_original_key"].get<bool>();
        if (got.consumed != wc || got.forwardOriginalKey != wf) {
            std::ostringstream oss;
            oss << label << " meta:";
            appendMetaDiff(oss, got, wc, wf);
            *err = oss.str();
            return false;
        }
    }
    return true;
}

}  // namespace

std::optional<std::vector<KeyEvent>> parseSequence(const nlohmann::json& caseObj, std::string* err) {
    if (!caseObj.contains("sequence") || !caseObj["sequence"].is_array()) {
        *err = "missing sequence array";
        return std::nullopt;
    }
    std::vector<KeyEvent> out;
    for (const auto& step : caseObj["sequence"]) {
        KeyEvent ev{};
        if (!parseKeyStep(step, &ev, err)) {
            return std::nullopt;
        }
        out.push_back(ev);
    }
    return out;
}

CorpusOutcome runCorpusCase(const nlohmann::json& c, CorpusRunStats* stats) {
    if (!c.is_object()) {
        return {false, "case is not a JSON object"};
    }

    const std::string id = c.value("id", std::string{});

    if (c.value("skip", false)) {
        if (stats) {
            ++stats->skipped;
        }
        return {true, std::nullopt};
    }

    if (id.empty()) {
        return {false, "missing id"};
    }

    const int schema = c.value("corpus_schema_version", -1);
    if (schema != 1) {
        return {false, id + ": unsupported corpus_schema_version (only 1 supported)"};
    }

    farolkey::config::RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.enableUserDictionary = false;  // corpus tests must not load the user's personal dict
    const std::string cfgName = c.value("config", "default");
    if (cfgName == "vni") {
        cfg.method = InputMethod::Vni;
    } else if (cfgName == "viqr") {
        cfg.method = InputMethod::Viqr;
    } else if (cfgName == "viqr_star") {
        cfg.method = InputMethod::ViqrStar;
    } else if (cfgName == "simple_telex") {
        cfg.method = InputMethod::SimpleTelex;
    } else if (cfgName == "simple_telex2") {
        cfg.method = InputMethod::SimpleTelex2;
    } else if (cfgName == "microsoft") {
        cfg.method = InputMethod::Microsoft;
    } else if (cfgName == "toc_ky") {
        cfg.method = InputMethod::TocKy;
    } else if (cfgName == "free_layout") {
        cfg.method = InputMethod::FreeLayout;
        // Standard test layout: VNI-style tones (1-5, 0-9) + consonant shortcuts
        cfg.freeLayout.tones.tone_sac      = '1';
        cfg.freeLayout.tones.tone_huyen    = '2';
        cfg.freeLayout.tones.tone_hoi      = '3';
        cfg.freeLayout.tones.tone_nga      = '4';
        cfg.freeLayout.tones.tone_nang     = '5';
        cfg.freeLayout.tones.diacritic_mui   = '6';
        cfg.freeLayout.tones.diacritic_breve = '8';
        cfg.freeLayout.tones.diacritic_moc   = '7';
        cfg.freeLayout.tones.diacritic_d     = '9';
        cfg.freeLayout.tones.remove          = '0';
        cfg.freeLayout.shortcuts = {
            {'f', "ph"}, {'j', "gi"}, {'w', "ng"},
        };
    } else if (cfgName == "default" || cfgName == "telex") {
        // default Telex
    } else {
        return {false, id + ": unknown config \"" + cfgName + "\""};
    }

    std::string perr;
    const auto seqOpt = parseSequence(c, &perr);
    if (!seqOpt.has_value()) {
        return {false, id + ": " + perr};
    }
    const std::vector<KeyEvent>& seq = *seqOpt;
    if (seq.empty()) {
        return {false, id + ": empty sequence"};
    }

    Engine engine(cfg);
    std::vector<ProcessResult> results;
    results.reserve(seq.size());
    for (const KeyEvent& ev : seq) {
        results.push_back(engine.processKey(ev));
    }

    const bool assertMeta =
        c.contains("expect") && c["expect"].is_object() ? c["expect"].value("assert_meta", false) : false;

    bool useTrace = false;
    if (c.contains("expect_trace")) {
        if (!c["expect_trace"].is_array()) {
            return {false, id + ": expect_trace must be array"};
        }
        if (c["expect_trace"].empty()) {
            return {false, id + ": expect_trace must not be empty (omit key instead)"};
        }
        useTrace = true;
    }

    if (useTrace) {
        const auto& trace = c["expect_trace"];
        if (trace.size() != seq.size()) {
            return {false, id + ": expect_trace size must match sequence size"};
        }
        for (std::size_t i = 0; i < seq.size(); ++i) {
            const std::string stepLabel = id + ": trace[" + std::to_string(i) + "]";
            std::string terr;
            if (!checkExpectBlock(stepLabel, results[i], trace[i], assertMeta, &terr)) {
                return {false, terr};
            }
        }
    } else {
        if (!c.contains("expect") || !c["expect"].is_object()) {
            return {false, id + ": missing expect object"};
        }
        const nlohmann::json& expectRoot = c["expect"];
        std::string terr;
        if (!checkExpectBlock(id + ": final", results.back(), expectRoot, assertMeta, &terr)) {
            return {false, terr};
        }
    }

    if (c.contains("expect_accumulated_commits") && !c["expect_accumulated_commits"].is_null()) {
        std::string acc;
        for (const ProcessResult& r : results) {
            acc += r.commit;
        }
        const std::string want = c["expect_accumulated_commits"].get<std::string>();
        if (acc != want) {
            return {false, id + ": expect_accumulated_commits want \"" + want + "\" got \"" + acc + "\""};
        }
    }

    if (stats) {
        ++stats->executed;
    }
    return {false, std::nullopt};
}

}  // namespace farolkey::test
