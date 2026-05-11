#include <cassert>

#include "cbakey/adapter/fcitx5/bridge.h"

using cbakey::adapter::fcitx5::Bridge;
using cbakey::core::InputMode;
using cbakey::core::KeyEvent;

int main() {
    Bridge bridge(cbakey::config::defaultConfig());

    bridge.handleKey(KeyEvent{.key = 'a'});
    bridge.handleKey(KeyEvent{.key = 'a'});
    bridge.handleKey(KeyEvent{.key = 's'});
    assert(bridge.preedit() == "ấ");

    const auto commitResult = bridge.handleKey(KeyEvent{.key = ' '});
    assert(commitResult.commit == "ấ ");
    const auto committed = bridge.drainCommitted();
    assert(committed.size() == 1);
    assert(committed[0] == "ấ ");
    assert(bridge.preedit().empty());

    bridge.handleKey(KeyEvent{.key = 'z', .ctrl = true, .alt = true});
    assert(bridge.inputMode() == InputMode::English);

    return 0;
}
