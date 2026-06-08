#include <cstdint>
#include <unordered_map>

namespace farolkey::core::charset {

// Windows-1258 (CP1258): Unicode NFC codepoint → CP1258 byte.
// Table populated in M22.3.
const std::unordered_map<char32_t, uint8_t>& cp1258Table() {
    static const std::unordered_map<char32_t, uint8_t> kTable{};
    return kTable;
}

}  // namespace farolkey::core::charset
