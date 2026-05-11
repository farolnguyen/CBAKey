#pragma once

#include <cstdint>
#include <string>

namespace cbakey::core {

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
};

struct ProcessResult {
    std::string preedit;
    std::string commit;
    bool consumed = false;
    /// If true, frontend should forward the original key event after applying commit/preedit.
    bool forwardOriginalKey = false;
};

}  // namespace cbakey::core
