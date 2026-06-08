#include <cstdint>
#include <unordered_map>

namespace farolkey::core::charset {

// TCVN3-1993 (ABC font encoding): Unicode NFC codepoint → TCVN3 byte.
// Table populated in M22.2.
const std::unordered_map<char32_t, uint8_t>& tcvn3Table() {
    static const std::unordered_map<char32_t, uint8_t> kTable{};
    return kTable;
}

}  // namespace farolkey::core::charset
