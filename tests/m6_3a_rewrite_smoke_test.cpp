#include <cassert>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"

using cbakey::config::defaultConfig;
using cbakey::core::Engine;
using cbakey::core::InputMethod;
using cbakey::core::KeyEvent;

int main() {
    const auto cfg = defaultConfig();
    const auto banSac = Engine::tryRewriteCommittedSyllable(cfg, "ban", KeyEvent{.key = 's'});
    assert(banSac.has_value());
    assert(*banSac == std::string("b\xC3\xA1n"));

    assert(!Engine::tryRewriteCommittedSyllable(cfg, "ban", KeyEvent{.key = 's', .ctrl = true}));
    assert(!Engine::tryRewriteCommittedSyllable(cfg, "xinchao", KeyEvent{.key = 's'}));

    cbakey::config::RuntimeConfig vni = cfg;
    vni.method = InputMethod::Vni;
    const auto thuoc6 = Engine::tryRewriteCommittedSyllable(vni, "thuoc", KeyEvent{.key = '6'});
    assert(thuoc6.has_value());
    assert(*thuoc6 != "thuoc");

    assert(!Engine::tryRewriteCommittedSyllable(cfg, "hello", KeyEvent{.key = 's'}));

    const auto chiuTone = Engine::tryRewriteCommittedSyllable(vni, "chiu", KeyEvent{.key = '2'});
    assert(chiuTone.has_value());
    assert(*chiuTone != "chiu");
    assert(*chiuTone != "chiu2");

    return 0;
}
