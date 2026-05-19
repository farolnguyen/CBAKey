#pragma once

#include <string>

#include "farolkey/config/config.h"
#include "farolkey/core/types.h"

namespace farolkey::adapter::fcitx5 {

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

PreeditPresentation choosePreeditPresentation(farolkey::config::Fcitx5PreeditMode mode,
                                              const PreeditCapabilitySnapshot& capabilities);
CommitDispatch adjustCommitForPresentation(PreeditPresentation presentation,
                                           farolkey::core::KeyAux aux,
                                           std::string commit);

}  // namespace farolkey::adapter::fcitx5
