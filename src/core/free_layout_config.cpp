#include "farolkey/core/free_layout_config.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>

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

namespace {

// Extract the value string for a JSON key on the same line.
// e.g. line = `  "tone_sac": "s",`  key = "tone_sac"  → "s"
std::string jsonStringValue(std::string_view line, std::string_view key) {
    const std::string needle = "\"" + std::string(key) + "\"";
    auto pos = line.find(needle);
    if (pos == std::string_view::npos) return {};
    pos += needle.size();
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == ':' || line[pos] == '\t')) ++pos;
    if (pos >= line.size() || line[pos] != '"') return {};
    ++pos;
    std::string result;
    while (pos < line.size() && line[pos] != '"') {
        if (line[pos] == '\\' && pos + 1 < line.size()) {
            ++pos;
            switch (line[pos]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                default:   result += line[pos]; break;
            }
        } else {
            result += line[pos];
        }
        ++pos;
    }
    return result;
}

// Set the appropriate FreeLayoutToneMap field from a key name + single-char value.
void applyToneField(FreeLayoutToneMap& t, const std::string& field, const std::string& val) {
    const char ch = (val.size() == 1) ? val[0] : '\0';
    if      (field == "tone_sac")        t.tone_sac      = ch;
    else if (field == "tone_huyen")      t.tone_huyen    = ch;
    else if (field == "tone_hoi")        t.tone_hoi      = ch;
    else if (field == "tone_nga")        t.tone_nga      = ch;
    else if (field == "tone_nang")       t.tone_nang     = ch;
    else if (field == "diacritic_mui")   t.diacritic_mui   = ch;
    else if (field == "diacritic_breve") t.diacritic_breve = ch;
    else if (field == "diacritic_moc")   t.diacritic_moc   = ch;
    else if (field == "diacritic_d")     t.diacritic_d     = ch;
    else if (field == "remove")          t.remove          = ch;
}

// All known tone field names.
static constexpr const char* kToneFields[] = {
    "tone_sac", "tone_huyen", "tone_hoi", "tone_nga", "tone_nang",
    "diacritic_mui", "diacritic_breve", "diacritic_moc", "diacritic_d", "remove",
    nullptr
};

}  // namespace

FreeLayoutConfig loadFreeLayoutConfig() {
    FreeLayoutConfig cfg = defaultFreeLayoutConfig();
    std::ifstream f(freeLayoutConfigPath());
    if (!f) return cfg;

    enum class Section { None, Tones, Shortcuts };
    Section section = Section::None;

    for (std::string line; std::getline(f, line); ) {
        const std::string_view sv(line);

        // Detect section change
        if (sv.find("\"tones\"") != std::string_view::npos &&
            sv.find(':') != std::string_view::npos) {
            section = Section::Tones;
            continue;
        }
        if (sv.find("\"shortcuts\"") != std::string_view::npos &&
            sv.find('[') != std::string_view::npos) {
            section = Section::Shortcuts;
            continue;
        }
        // Closing brace/bracket — back to none
        const auto stripped = sv.find_first_not_of(" \t");
        if (stripped != std::string_view::npos &&
            (sv[stripped] == '}' || sv[stripped] == ']')) {
            if (section != Section::None) section = Section::None;
            continue;
        }

        if (section == Section::Tones) {
            for (const char* const* fp = kToneFields; *fp; ++fp) {
                if (sv.find(*fp) != std::string_view::npos) {
                    const std::string val = jsonStringValue(sv, *fp);
                    applyToneField(cfg.tones, *fp, val);
                    break;
                }
            }
        } else if (section == Section::Shortcuts) {
            // Each shortcut line looks like: {"key": "f", "output": "ph"}
            if (sv.find("\"key\"") == std::string_view::npos) continue;
            const std::string k = jsonStringValue(sv, "key");
            const std::string o = jsonStringValue(sv, "output");
            if (k.size() == 1 && !o.empty())
                cfg.shortcuts.push_back({k[0], o});
        }
    }
    return cfg;
}

void saveFreeLayoutConfig(const FreeLayoutConfig& cfg) {
    const std::string path = freeLayoutConfigPath();
    const std::string tmp  = path + ".tmp";

    auto keyStr = [](char k) -> std::string {
        if (!k) return "";
        if (k == '"' || k == '\\') return std::string(1, '\\') + k;
        return std::string(1, k);
    };

    // Simple JSON escape for output strings
    auto escape = [](const std::string& s) {
        std::string r;
        for (char c : s) {
            if (c == '"')  { r += "\\\""; }
            else if (c == '\\') { r += "\\\\"; }
            else           { r += c; }
        }
        return r;
    };

    std::ofstream f(tmp);
    if (!f) return;

    f << "{\n  \"tones\": {\n";
    const FreeLayoutToneMap& t = cfg.tones;
    f << "    \"tone_sac\": \""        << keyStr(t.tone_sac)      << "\",\n";
    f << "    \"tone_huyen\": \""      << keyStr(t.tone_huyen)    << "\",\n";
    f << "    \"tone_hoi\": \""        << keyStr(t.tone_hoi)      << "\",\n";
    f << "    \"tone_nga\": \""        << keyStr(t.tone_nga)      << "\",\n";
    f << "    \"tone_nang\": \""       << keyStr(t.tone_nang)     << "\",\n";
    f << "    \"diacritic_mui\": \""   << keyStr(t.diacritic_mui)   << "\",\n";
    f << "    \"diacritic_breve\": \"" << keyStr(t.diacritic_breve) << "\",\n";
    f << "    \"diacritic_moc\": \""   << keyStr(t.diacritic_moc)   << "\",\n";
    f << "    \"diacritic_d\": \""     << keyStr(t.diacritic_d)     << "\",\n";
    f << "    \"remove\": \""          << keyStr(t.remove)          << "\"\n";
    f << "  },\n  \"shortcuts\": [\n";

    for (std::size_t i = 0; i < cfg.shortcuts.size(); ++i) {
        const auto& r = cfg.shortcuts[i];
        f << "    {\"key\": \"" << keyStr(r.key)
          << "\", \"output\": \"" << escape(r.output) << "\"}";
        if (i + 1 < cfg.shortcuts.size()) f << ",";
        f << "\n";
    }
    f << "  ]\n}\n";
    f.close();

    std::rename(tmp.c_str(), path.c_str());
}

}  // namespace farolkey::core
