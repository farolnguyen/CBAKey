#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace cbakey::core {

/// One entry in the user dictionary.
/// trigger: ASCII key sequence the user types (e.g. "btv", "ko")
/// expansion: UTF-8 output string (e.g. "Ban Tổ chức", "không")
/// force: if true, expansion overrides even when trigger is a valid Vietnamese syllable
struct UserDictEntry {
    std::string trigger;
    std::string expansion;
    bool force = false;
};

/// Loaded and indexed user dictionary.
/// Lookup is O(1) by trigger string.
class UserDict {
public:
    UserDict() = default;

    /// Load from a JSON file (one object per line, or a JSON array).
    /// Returns the number of entries successfully loaded.
    /// Logs and skips malformed entries; does not throw.
    static UserDict loadFromFile(const std::string& path);

    /// Look up a trigger. Returns pointer to entry if found, nullptr otherwise.
    const UserDictEntry* lookup(const std::string& trigger) const;

    bool empty() const { return entries_.empty(); }
    std::size_t size() const { return entries_.size(); }

private:
    std::unordered_map<std::string, UserDictEntry> entries_;
};

}  // namespace cbakey::core
