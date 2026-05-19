#pragma once

#include <cstdint>
#include <string>

namespace farolkey::core {

enum class InputMode {
    English,
    Vietnamese
};

enum class InputMethod {
    Telex,
    Vni
};

/// Non-character keys from the frontend (Fcitx5). Named KeyAux to avoid system macro clashes.
enum class KeyAux : std::uint8_t {
    None = 0,
    Enter,
    Tab,
    Left,
    Right,
    Up,
    Down,
    Home,
    End,
    DeleteForward,
};

struct KeyEvent {
    char key = '\0';
    KeyAux aux = KeyAux::None;
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    /// True when the key originated from the numeric keypad (Fcitx KP_* keysyms).
    bool key_from_keypad = false;
};

struct ProcessResult {
    std::string preedit;
    std::string commit;
    bool consumed = false;
    /// If true, frontend should forward the original key event after applying commit/preedit.
    bool forwardOriginalKey = false;
    /// M15: commit came from a Smart Template expansion (may be multi-line).
    /// Adapter should prefer wl-copy + auto-paste over inline commitString.
    bool smartTemplateExpansion = false;
};

}  // namespace farolkey::core
