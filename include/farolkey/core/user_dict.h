#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace farolkey::core {

/// In which input mode an abbreviation is active.
/// Vi   — only when composing Vietnamese (default; preserves M8 behaviour)
/// En   — only in English passthrough mode
/// Both — active in both modes
enum class AbbrevMode { Vi, En, Both };

/// One entry in the user dictionary / abbreviation table.
/// trigger:     ASCII key sequence the user types (e.g. "btv", "ko", "doex")
/// expansion:   UTF-8 output string (e.g. "Ban Tổ chức", "không", "docker exec …")
/// abbrev_mode: in which input mode this entry fires (M13)
/// force:       reserved for future use (was in M8 schema, kept for compatibility)
struct UserDictEntry {
    std::string trigger;
    std::string expansion;
    AbbrevMode  abbrev_mode = AbbrevMode::Vi;
    bool        force       = false;
};

/// Loaded and indexed user dictionary / abbreviation table.
/// Lookup is O(1) by trigger string.
class UserDict {
public:
    UserDict() = default;

    /// Load from a JSON file (one object per line, or a JSON array).
    /// Logs and skips malformed entries; does not throw.
    static UserDict loadFromFile(const std::string& path);

    /// Save all entries to \p path (JSONL format).
    /// Backs up the existing file to <path>.bak before writing.
    /// Returns true on success.
    bool saveToFile(const std::string& path) const;

    /// Look up a trigger. Returns pointer to entry if found, nullptr otherwise.
    const UserDictEntry* lookup(const std::string& trigger) const;

    /// Add or update an entry. Returns true if it replaced an existing trigger.
    bool upsert(UserDictEntry entry);

    /// Remove an entry by trigger. Returns true if it was present.
    bool remove(const std::string& trigger);

    bool empty() const { return entries_.empty(); }
    std::size_t size() const { return entries_.size(); }

    /// Ordered list of triggers (insertion order preserved via ordered_keys_).
    const std::vector<std::string>& keys() const { return ordered_keys_; }

private:
    std::unordered_map<std::string, UserDictEntry> entries_;
    std::vector<std::string> ordered_keys_;
};

}  // namespace farolkey::core
