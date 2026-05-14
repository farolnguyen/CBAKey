#include "cbakey/core/user_dict.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace cbakey::core {

namespace {

std::string extractJsonString(std::string_view line, std::string_view key) {
    const std::string needle = "\"" + std::string(key) + "\"";
    auto pos = line.find(needle);
    if (pos == std::string_view::npos) return {};
    pos += needle.size();
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t' || line[pos] == ':'))
        ++pos;
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
                case 'u': {
                    if (pos + 4 < line.size()) {
                        char hex[5] = {
                            static_cast<char>(line[pos+1]), static_cast<char>(line[pos+2]),
                            static_cast<char>(line[pos+3]), static_cast<char>(line[pos+4]), '\0'};
                        char* end = nullptr;
                        const unsigned long cp = std::strtoul(hex, &end, 16);
                        if (end == hex + 4) {
                            if      (cp <= 0x7F)  { result += static_cast<char>(cp); }
                            else if (cp <= 0x7FF) { result += static_cast<char>(0xC0|((cp>>6)&0x1F));
                                                    result += static_cast<char>(0x80|(cp&0x3F)); }
                            else                  { result += static_cast<char>(0xE0|((cp>>12)&0x0F));
                                                    result += static_cast<char>(0x80|((cp>>6)&0x3F));
                                                    result += static_cast<char>(0x80|(cp&0x3F)); }
                            pos += 4;
                            break;
                        }
                    }
                    result += '\\'; result += 'u';
                    break;
                }
                default: result += '\\'; result += line[pos]; break;
            }
        } else {
            result += line[pos];
        }
        ++pos;
    }
    return result;
}

bool extractJsonBool(std::string_view line, std::string_view key) {
    const std::string needle = "\"" + std::string(key) + "\"";
    auto pos = line.find(needle);
    if (pos == std::string_view::npos) return false;
    pos += needle.size();
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t' || line[pos] == ':'))
        ++pos;
    return line.size() - pos >= 4 && line.substr(pos, 4) == "true";
}

AbbrevMode parseAbbrevMode(std::string_view s) {
    if (s == "en")   return AbbrevMode::En;
    if (s == "both") return AbbrevMode::Both;
    return AbbrevMode::Vi;  // "vi" or unknown → default
}

const char* abbrevModeStr(AbbrevMode m) {
    switch (m) {
        case AbbrevMode::En:   return "en";
        case AbbrevMode::Both: return "both";
        default:               return "vi";
    }
}

/// Escape a UTF-8 string for embedding in a JSON string value.
std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (unsigned char c : s) {
        if      (c == '"')  { out += "\\\""; }
        else if (c == '\\') { out += "\\\\"; }
        else if (c == '\n') { out += "\\n";  }
        else if (c == '\t') { out += "\\t";  }
        else                { out += static_cast<char>(c); }
    }
    return out;
}

}  // namespace

// static
UserDict UserDict::loadFromFile(const std::string& path) {
    UserDict dict;
    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) return dict;

    char buf[4096];
    while (std::fgets(buf, sizeof(buf), f)) {
        std::string_view line(buf);
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front()))) line.remove_prefix(1);
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))  line.remove_suffix(1);
        if (line.empty() || line[0] == '/' || line[0] == '#') continue;
        if (line == "[" || line == "]") continue;
        if (!line.empty() && line.back() == ',') line.remove_suffix(1);

        const std::string trigger   = extractJsonString(line, "trigger");
        const std::string expansion = extractJsonString(line, "expansion");
        if (trigger.empty() || expansion.empty()) continue;

        const std::string modeStr = extractJsonString(line, "abbrev_mode");
        const bool force          = extractJsonBool(line, "force");

        UserDictEntry entry{trigger, expansion, parseAbbrevMode(modeStr), force};
        if (dict.entries_.find(trigger) == dict.entries_.end()) {
            dict.ordered_keys_.push_back(trigger);
        }
        dict.entries_[trigger] = std::move(entry);
    }
    std::fclose(f);
    return dict;
}

bool UserDict::saveToFile(const std::string& path) const {
    // Backup existing file.
    const std::string bakPath = path + ".bak";
    if (FILE* src = std::fopen(path.c_str(), "r")) {
        std::fclose(src);
        std::rename(path.c_str(), bakPath.c_str());
    }

    // Write to a temp file, then rename atomically.
    const std::string tmpPath = path + ".tmp";
    FILE* f = std::fopen(tmpPath.c_str(), "w");
    if (!f) return false;

    for (const std::string& key : ordered_keys_) {
        const auto it = entries_.find(key);
        if (it == entries_.end()) continue;
        const UserDictEntry& e = it->second;
        std::fprintf(f, "{\"trigger\":\"%s\",\"expansion\":\"%s\",\"abbrev_mode\":\"%s\"}\n",
                     jsonEscape(e.trigger).c_str(),
                     jsonEscape(e.expansion).c_str(),
                     abbrevModeStr(e.abbrev_mode));
    }
    std::fclose(f);
    return std::rename(tmpPath.c_str(), path.c_str()) == 0;
}

const UserDictEntry* UserDict::lookup(const std::string& trigger) const {
    const auto it = entries_.find(trigger);
    return it == entries_.end() ? nullptr : &it->second;
}

bool UserDict::upsert(UserDictEntry entry) {
    const bool replaced = entries_.find(entry.trigger) != entries_.end();
    if (!replaced) ordered_keys_.push_back(entry.trigger);
    entries_[entry.trigger] = std::move(entry);
    return replaced;
}

bool UserDict::remove(const std::string& trigger) {
    if (entries_.erase(trigger) == 0) return false;
    ordered_keys_.erase(
        std::remove(ordered_keys_.begin(), ordered_keys_.end(), trigger),
        ordered_keys_.end());
    return true;
}

}  // namespace cbakey::core
