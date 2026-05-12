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

    Bridge smartBoundaryBridge(cbakey::config::defaultConfig());
    for (const char k : {'x', 'i', 'n', 'c', 'h'}) {
        const auto r = smartBoundaryBridge.handleKey(KeyEvent{.key = k});
        assert(r.commit.empty());
    }
    const auto splitResult = smartBoundaryBridge.handleKey(KeyEvent{.key = 'a'});
    assert(splitResult.commit == "xin");
    assert(splitResult.preedit == "cha");
    assert(smartBoundaryBridge.preedit() == "cha");
    const auto splitCommitted = smartBoundaryBridge.drainCommitted();
    assert(splitCommitted.size() == 1);
    assert(splitCommitted[0] == "xin");
    const auto suffixResult = smartBoundaryBridge.handleKey(KeyEvent{.key = 'o'});
    assert(suffixResult.commit.empty());
    assert(suffixResult.preedit == "chao");
    assert(smartBoundaryBridge.preedit() == "chao");

    bridge.handleKey(KeyEvent{.key = 'z', .ctrl = true, .alt = true});
    assert(bridge.inputMode() == InputMode::English);

    return 0;
}
