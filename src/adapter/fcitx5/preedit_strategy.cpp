#include "farolkey/adapter/fcitx5/preedit_strategy.h"

namespace farolkey::adapter::fcitx5 {

PreeditPresentation choosePreeditPresentation(farolkey::config::Fcitx5PreeditMode mode,
                                              const PreeditCapabilitySnapshot& capabilities) {
    switch (mode) {
        case farolkey::config::Fcitx5PreeditMode::Client:
            return PreeditPresentation::Client;
        case farolkey::config::Fcitx5PreeditMode::Panel:
            return PreeditPresentation::Panel;
        case farolkey::config::Fcitx5PreeditMode::Auto:
            return capabilities.supports_client_preedit ? PreeditPresentation::Client
                                                        : PreeditPresentation::Panel;
    }
    return PreeditPresentation::Panel;
}

CommitDispatch adjustCommitForPresentation(PreeditPresentation presentation,
                                           farolkey::core::KeyAux aux,
                                           std::string commit) {
    (void)presentation;
    CommitDispatch dispatch{.commit = std::move(commit), .forward_original_key = false};
    // Always forward the original Enter/Tab key instead of embedding it in the commit
    // string. Apps that use Enter for "send" (Teams, Slack) or Tab for completion
    // (terminals) need the actual key event — receiving '\n'/'\t' via commitString
    // is not equivalent. This applies to both Client and Panel preedit modes.
    if (aux == farolkey::core::KeyAux::Enter &&
        !dispatch.commit.empty() && dispatch.commit.back() == '\n') {
        dispatch.commit.pop_back();
        dispatch.forward_original_key = true;
    }
    if (aux == farolkey::core::KeyAux::Tab &&
        !dispatch.commit.empty() && dispatch.commit.back() == '\t') {
        dispatch.commit.pop_back();
        dispatch.forward_original_key = true;
    }
    return dispatch;
}

}  // namespace farolkey::adapter::fcitx5
