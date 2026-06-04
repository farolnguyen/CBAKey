#pragma once

#include <string>
#include <vector>

namespace farolkey::core {

// 10 fixed tone/diacritic actions; each mapped to a user-chosen key ('\0' = disabled).
struct FreeLayoutToneMap {
    char tone_sac     = '\0';  // Sắc  ´  (applyTone 's')
    char tone_huyen   = '\0';  // Huyền `  (applyTone 'f')
    char tone_hoi     = '\0';  // Hỏi  ̉  (applyTone 'r')
    char tone_nga     = '\0';  // Ngã  ~  (applyTone 'x')
    char tone_nang    = '\0';  // Nặng .  (applyTone 'j')
    char diacritic_mui   = '\0';  // Mũ    → â ê ô  (applyVniTransform '6')
    char diacritic_breve = '\0';  // Breve → ă      (applyVniTransform '8')
    char diacritic_moc   = '\0';  // Móc   → ơ ư    (applyVniTransform '7')
    char diacritic_d     = '\0';  // D gạch d→đ     (applyVniTransform '9')
    char remove          = '\0';  // Xóa toàn bộ dấu

    bool operator==(const FreeLayoutToneMap& o) const {
        return tone_sac == o.tone_sac && tone_huyen == o.tone_huyen &&
               tone_hoi == o.tone_hoi && tone_nga == o.tone_nga &&
               tone_nang == o.tone_nang && diacritic_mui == o.diacritic_mui &&
               diacritic_breve == o.diacritic_breve && diacritic_moc == o.diacritic_moc &&
               diacritic_d == o.diacritic_d && remove == o.remove;
    }
};

struct FreeLayoutRule {
    char key = '\0';
    std::string output;  // UTF-8 string pushed into buffer when key is pressed

    bool operator==(const FreeLayoutRule& o) const {
        return key == o.key && output == o.output;
    }
};

struct FreeLayoutConfig {
    FreeLayoutToneMap tones;
    std::vector<FreeLayoutRule> shortcuts;

    bool operator==(const FreeLayoutConfig& o) const {
        return tones == o.tones && shortcuts == o.shortcuts;
    }
};

// Returns default Free Layout config (all keys unset, empty shortcuts).
FreeLayoutConfig defaultFreeLayoutConfig();

// Load from ~/.config/farolkey/free_layout.json.
// Returns defaultFreeLayoutConfig() if file missing or malformed.
FreeLayoutConfig loadFreeLayoutConfig();

// Returns the default XDG path for free_layout.json.
std::string freeLayoutConfigPath();

// Save config to the default path atomically.
void saveFreeLayoutConfig(const FreeLayoutConfig& cfg);

}  // namespace farolkey::core
