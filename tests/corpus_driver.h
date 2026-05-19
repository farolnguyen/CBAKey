#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "farolkey/core/types.h"

namespace farolkey::test {

struct CorpusRunStats {
    int skipped = 0;
    int executed = 0;
};

struct CorpusOutcome {
    bool skipped = false;
    std::optional<std::string> error;
};

/// Parses `sequence` into KeyEvents; returns nullopt and sets err on failure.
std::optional<std::vector<farolkey::core::KeyEvent>> parseSequence(const nlohmann::json& caseObj,
                                                                 std::string* err);

/// Runs one corpus case: builds `Engine` from `config` field. Skip does not count as error.
CorpusOutcome runCorpusCase(const nlohmann::json& caseObj, CorpusRunStats* stats = nullptr);

}  // namespace farolkey::test
