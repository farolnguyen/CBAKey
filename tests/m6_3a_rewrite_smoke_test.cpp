#include <cassert>

#include "farolkey/config/config.h"
#include "farolkey/core/engine.h"

using farolkey::config::defaultConfig;
using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;

int main() {
    const auto cfg = defaultConfig();
    const auto banSac = Engine::tryRewriteCommittedSyllable(cfg, "ban", KeyEvent{.key = 's'});
    assert(banSac.has_value());
    assert(*banSac == std::string("b\xC3\xA1n"));

    assert(!Engine::tryRewriteCommittedSyllable(cfg, "ban", KeyEvent{.key = 's', .ctrl = true}));
    assert(!Engine::tryRewriteCommittedSyllable(cfg, "xinchao", KeyEvent{.key = 's'}));

    farolkey::config::RuntimeConfig vni = cfg;
    vni.method = InputMethod::Vni;
    const auto thuoc6 = Engine::tryRewriteCommittedSyllable(vni, "thuoc", KeyEvent{.key = '6'});
    assert(thuoc6.has_value());
    assert(*thuoc6 != "thuoc");

    assert(!Engine::tryRewriteCommittedSyllable(cfg, "hello", KeyEvent{.key = 's'}));

    const auto chiuTone = Engine::tryRewriteCommittedSyllable(vni, "chiu", KeyEvent{.key = '2'});
    assert(chiuTone.has_value());
    assert(*chiuTone != "chiu");
    assert(*chiuTone != "chiu2");

    // C1 + "ua+coda" fix: committed token already has correct form, z strips whole syllable.
    // "chuẩn" (c,h,u,ẩ,n) z-strip → "chuan"
    const auto chuanZ = Engine::tryRewriteCommittedSyllable(cfg, "chu\xe1\xba\xa9n", KeyEvent{.key = 'z'});
    assert(chuanZ.has_value());
    assert(*chuanZ == "chuan");

    // "tuần" (t,u,ầ,n) z-strip → "tuan"
    const auto tuanZ = Engine::tryRewriteCommittedSyllable(cfg, "tu\xe1\xba\xa7n", KeyEvent{.key = 'z'});
    assert(tuanZ.has_value());
    assert(*tuanZ == "tuan");

    // Applying tone to plain "tuan" committed: should place tone on 'a' (second vowel), not 'u'.
    // Telex 'f' (huyền) on "tuan" → "tuàn" (à at position 2), not "tụan".
    const auto tuanHuyen = Engine::tryRewriteCommittedSyllable(cfg, "tuan", KeyEvent{.key = 'f'});
    assert(tuanHuyen.has_value());
    assert(*tuanHuyen != "tuan");
    // Tone must NOT be on 'u' (no ù or ụ at position 1).
    assert(tuanHuyen->find('\xC3') == std::string::npos || (*tuanHuyen)[0] != 't' || (*tuanHuyen)[1] != '\xC3');
    // Result begins with "tu" (u still toneless).
    assert(tuanHuyen->substr(0, 2) == "tu");

    return 0;
}
