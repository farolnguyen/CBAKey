#include <cassert>

#include "farolkey/adapter/fcitx5/preedit_strategy.h"

using farolkey::adapter::fcitx5::PreeditCapabilitySnapshot;
using farolkey::adapter::fcitx5::CommitDispatch;
using farolkey::adapter::fcitx5::PreeditPresentation;
using farolkey::adapter::fcitx5::adjustCommitForPresentation;
using farolkey::adapter::fcitx5::choosePreeditPresentation;
using farolkey::core::KeyAux;
using farolkey::config::Fcitx5PreeditMode;

int main() {
    assert(choosePreeditPresentation(Fcitx5PreeditMode::Auto,
                                     PreeditCapabilitySnapshot{.supports_client_preedit = true}) ==
           PreeditPresentation::Client);
    assert(choosePreeditPresentation(Fcitx5PreeditMode::Auto,
                                     PreeditCapabilitySnapshot{.supports_client_preedit = false}) ==
           PreeditPresentation::Panel);
    assert(choosePreeditPresentation(Fcitx5PreeditMode::Client,
                                     PreeditCapabilitySnapshot{.supports_client_preedit = false}) ==
           PreeditPresentation::Client);
    assert(choosePreeditPresentation(Fcitx5PreeditMode::Panel,
                                     PreeditCapabilitySnapshot{.supports_client_preedit = true}) ==
           PreeditPresentation::Panel);

    const CommitDispatch clientEnter =
        adjustCommitForPresentation(PreeditPresentation::Client, KeyAux::Enter, "a\n");
    assert(clientEnter.commit == "a");
    assert(clientEnter.forward_original_key);

    const CommitDispatch panelEnter =
        adjustCommitForPresentation(PreeditPresentation::Panel, KeyAux::Enter, "a\n");
    assert(panelEnter.commit == "a\n");
    assert(!panelEnter.forward_original_key);

    const CommitDispatch clientSpace =
        adjustCommitForPresentation(PreeditPresentation::Client, KeyAux::None, "a ");
    assert(clientSpace.commit == "a ");
    assert(!clientSpace.forward_original_key);

    return 0;
}
