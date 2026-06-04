#include "farolkey/core/free_layout_config.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace farolkey::core {

std::string freeLayoutConfigPath() {
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    std::string base = (xdg && xdg[0])
        ? std::string(xdg)
        : std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/.config";
    return base + "/farolkey/free_layout.json";
}

FreeLayoutConfig defaultFreeLayoutConfig() {
    return {};
}

static char parseKey(const nlohmann::json& j, const char* field) {
    if (!j.contains(field)) return '\0';
    const auto& v = j[field];
    if (!v.is_string()) return '\0';
    const std::string s = v.get<std::string>();
    if (s.size() != 1) return '\0';
    return s[0];
}

FreeLayoutConfig loadFreeLayoutConfig() {
    FreeLayoutConfig cfg = defaultFreeLayoutConfig();
    std::ifstream f(freeLayoutConfigPath());
    if (!f) return cfg;
    try {
        nlohmann::json j;
        f >> j;

        if (j.contains("tones") && j["tones"].is_object()) {
            const auto& t = j["tones"];
            cfg.tones.tone_sac      = parseKey(t, "tone_sac");
            cfg.tones.tone_huyen    = parseKey(t, "tone_huyen");
            cfg.tones.tone_hoi      = parseKey(t, "tone_hoi");
            cfg.tones.tone_nga      = parseKey(t, "tone_nga");
            cfg.tones.tone_nang     = parseKey(t, "tone_nang");
            cfg.tones.diacritic_mui   = parseKey(t, "diacritic_mui");
            cfg.tones.diacritic_breve = parseKey(t, "diacritic_breve");
            cfg.tones.diacritic_moc   = parseKey(t, "diacritic_moc");
            cfg.tones.diacritic_d     = parseKey(t, "diacritic_d");
            cfg.tones.remove          = parseKey(t, "remove");
        }

        if (j.contains("shortcuts") && j["shortcuts"].is_array()) {
            for (const auto& item : j["shortcuts"]) {
                if (!item.is_object()) continue;
                if (!item.contains("key") || !item.contains("output")) continue;
                const std::string ks = item["key"].is_string()
                    ? item["key"].get<std::string>() : "";
                if (ks.size() != 1) continue;
                const std::string out = item["output"].is_string()
                    ? item["output"].get<std::string>() : "";
                if (out.empty()) continue;
                cfg.shortcuts.push_back({ks[0], out});
            }
        }
    } catch (...) {
        return defaultFreeLayoutConfig();
    }
    return cfg;
}

static std::string keyStr(char k) {
    return k ? std::string(1, k) : "";
}

void saveFreeLayoutConfig(const FreeLayoutConfig& cfg) {
    nlohmann::json j;

    nlohmann::json t = nlohmann::json::object();
    t["tone_sac"]        = keyStr(cfg.tones.tone_sac);
    t["tone_huyen"]      = keyStr(cfg.tones.tone_huyen);
    t["tone_hoi"]        = keyStr(cfg.tones.tone_hoi);
    t["tone_nga"]        = keyStr(cfg.tones.tone_nga);
    t["tone_nang"]       = keyStr(cfg.tones.tone_nang);
    t["diacritic_mui"]   = keyStr(cfg.tones.diacritic_mui);
    t["diacritic_breve"] = keyStr(cfg.tones.diacritic_breve);
    t["diacritic_moc"]   = keyStr(cfg.tones.diacritic_moc);
    t["diacritic_d"]     = keyStr(cfg.tones.diacritic_d);
    t["remove"]          = keyStr(cfg.tones.remove);
    j["tones"] = t;

    nlohmann::json arr = nlohmann::json::array();
    for (const auto& r : cfg.shortcuts) {
        nlohmann::json item;
        item["key"]    = std::string(1, r.key);
        item["output"] = r.output;
        arr.push_back(item);
    }
    j["shortcuts"] = arr;

    const std::string path = freeLayoutConfigPath();
    const std::string tmp  = path + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f) return;
        f << j.dump(2) << "\n";
    }
    std::rename(tmp.c_str(), path.c_str());
}

}  // namespace farolkey::core
