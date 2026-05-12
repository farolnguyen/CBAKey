#pragma once

#include <string>

#include "cbakey/config/config.h"
#include "cbakey/core/types.h"

namespace cbakey::adapter::fcitx5 {

enum class PreeditPresentation {
    Client,
    Panel,
};

struct PreeditCapabilitySnapshot {
    bool supports_client_preedit = false;
};

struct CommitDispatch {
    std::string commit;
    bool forward_original_key = false;
};

PreeditPresentation choosePreeditPresentation(cbakey::config::Fcitx5PreeditMode mode,
                                              const PreeditCapabilitySnapshot& capabilities);
CommitDispatch adjustCommitForPresentation(PreeditPresentation presentation,
                                           cbakey::core::KeyAux aux,
                                           std::string commit);

}  // namespace cbakey::adapter::fcitx5
