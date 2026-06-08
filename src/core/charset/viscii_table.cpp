#include <cstdint>
#include <unordered_map>

namespace farolkey::core::charset {

// VISCII-1.1: Unicode NFC codepoint → VISCII byte.
// Table populated in M22.4.
const std::unordered_map<char32_t, uint8_t>& visciiTable() {
    static const std::unordered_map<char32_t, uint8_t> kTable{};
    return kTable;
}

}  // namespace farolkey::core::charset
