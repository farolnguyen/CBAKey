#include "cbakey/core/user_dict.h"

#include <cctype>
#include <cstdio>
#include <string>
#include <string_view>

namespace cbakey::core {

namespace {

/// Minimal JSON value extractor — no external dependency.
/// Returns the string value for `"key"` in a flat JSON object line.
/// Only handles string values; returns empty string if not found or on error.
std::string extractJsonString(std::string_view line, std::string_view key) {
    // Find `"key"` then `:` then `"value"`.
    const std::string needle = "\"" + std::string(key) + "\"";
    auto pos = line.find(needle);
    if (pos == std::string_view::npos) {
        return {};
    }
    pos += needle.size();
    // skip whitespace and colon
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t' || line[pos] == ':')) {
        ++pos;
    }
    if (pos >= line.size() || line[pos] != '"') {
        return {};
    }
    ++pos;  // skip opening quote
    std::string result;
    while (pos < line.size() && line[pos] != '"') {
        if (line[pos] == '\\' && pos + 1 < line.size()) {
            ++pos;
            switch (line[pos]) {
                case '"':
                    result += '"';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 't':
                    result += '\t';
                    break;
                case 'u': {
                    // \uXXXX — decode 4 hex digits into UTF-8
                    if (pos + 4 < line.size()) {
                        char hex[5] = {
                            static_cast<char>(line[pos + 1]),
                            static_cast<char>(line[pos + 2]),
                            static_cast<char>(line[pos + 3]),
                            static_cast<char>(line[pos + 4]),
                            '\0'};
                        char* end = nullptr;
                        const unsigned long cp = std::strtoul(hex, &end, 16);
                        if (end == hex + 4) {
                            // Encode as UTF-8
                            if (cp <= 0x7F) {
                                result += static_cast<char>(cp);
                            } else if (cp <= 0x7FF) {
                                result += static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
                                result += static_cast<char>(0x80 | (cp & 0x3F));
                            } else {
                                result += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
                                result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                                result += static_cast<char>(0x80 | (cp & 0x3F));
                            }
                            pos += 4;
                            break;
                        }
                    }
                    result += '\\';
                    result += 'u';
                    break;
                }
                default:
                    result += '\\';
                    result += line[pos];
                    break;
            }
        } else {
            result += line[pos];
        }
        ++pos;
    }
    return result;
}

/// Extract a boolean value for `"key"` — returns false if not found or not "true".
bool extractJsonBool(std::string_view line, std::string_view key) {
    const std::string needle = "\"" + std::string(key) + "\"";
    auto pos = line.find(needle);
    if (pos == std::string_view::npos) {
        return false;
    }
    pos += needle.size();
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t' || line[pos] == ':')) {
        ++pos;
    }
    return line.size() - pos >= 4 && line.substr(pos, 4) == "true";
}

}  // namespace

// static
UserDict UserDict::loadFromFile(const std::string& path) {
    UserDict dict;
    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) {
        // File absent is not an error — user simply hasn't created one yet.
        return dict;
    }

    char buf[4096];
    while (std::fgets(buf, sizeof(buf), f)) {
        std::string_view line(buf);
        // Trim leading/trailing whitespace
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front()))) {
            line.remove_prefix(1);
        }
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) {
            line.remove_suffix(1);
        }

        // Skip blank lines and comment-like lines
        if (line.empty() || line[0] == '/' || line[0] == '#') {
            continue;
        }
        // Skip array brackets
        if (line == "[" || line == "]") {
            continue;
        }
        // Strip trailing comma (array style)
        if (!line.empty() && line.back() == ',') {
            line.remove_suffix(1);
        }

        const std::string trigger = extractJsonString(line, "trigger");
        const std::string expansion = extractJsonString(line, "expansion");
        if (trigger.empty() || expansion.empty()) {
            continue;
        }
        const bool force = extractJsonBool(line, "force");
        dict.entries_[trigger] = UserDictEntry{trigger, expansion, force};
    }
    std::fclose(f);
    return dict;
}

const UserDictEntry* UserDict::lookup(const std::string& trigger) const {
    const auto it = entries_.find(trigger);
    return it == entries_.end() ? nullptr : &it->second;
}

}  // namespace cbakey::core
