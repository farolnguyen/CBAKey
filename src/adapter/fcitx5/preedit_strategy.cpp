#include "cbakey/adapter/fcitx5/preedit_strategy.h"

namespace cbakey::adapter::fcitx5 {

PreeditPresentation choosePreeditPresentation(cbakey::config::Fcitx5PreeditMode mode,
                                              const PreeditCapabilitySnapshot& capabilities) {
    switch (mode) {
        case cbakey::config::Fcitx5PreeditMode::Client:
            return PreeditPresentation::Client;
        case cbakey::config::Fcitx5PreeditMode::Panel:
            return PreeditPresentation::Panel;
        case cbakey::config::Fcitx5PreeditMode::Auto:
            return capabilities.supports_client_preedit ? PreeditPresentation::Client
                                                        : PreeditPresentation::Panel;
    }
    return PreeditPresentation::Panel;
}

CommitDispatch adjustCommitForPresentation(PreeditPresentation presentation,
                                           cbakey::core::KeyAux aux,
                                           std::string commit) {
    CommitDispatch dispatch{.commit = std::move(commit), .forward_original_key = false};
    if (presentation == PreeditPresentation::Client && aux == cbakey::core::KeyAux::Enter &&
        !dispatch.commit.empty() && dispatch.commit.back() == '\n') {
        dispatch.commit.pop_back();
        dispatch.forward_original_key = true;
    }
    return dispatch;
}

}  // namespace cbakey::adapter::fcitx5
