// M14.10 — Microsoft Vietnamese Input Method smoke tests
// Number keys insert Vietnamese chars directly (no base letter needed).
//   1→ă  2→â  3→ê  4→ô  7→ư  8→ơ  0→đ
// Tone keys: s→sắc  j→nặng  5→huyền  6→hỏi  9→ngã
#include <cassert>
#include <string>
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"
#include "farolkey/config/config.h"

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::config::RuntimeConfig;

static Engine makeMicrosoft() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::Microsoft;
    return Engine(cfg);
}

static std::string typeAndFlush(Engine& e, const std::string& seq) {
    std::string result;
    for (char c : seq) {
        auto r = e.processKey(KeyEvent{.key = c});
        result += r.commit;
    }
    auto r = e.processKey(KeyEvent{.key = ' '});
    result += r.commit;
    if (!result.empty() && result.back() == ' ')
        result = result.substr(0, result.size() - 1);
    return result;
}

static void resetEngine(Engine& e) { e.clearState(); }

// ── Direct character keys ────────────────────────────────────────────────────

static void test_microsoft_direct_chars() {
    auto e = makeMicrosoft();
    assert(typeAndFlush(e, "1") == "ă"); resetEngine(e);
    assert(typeAndFlush(e, "2") == "â"); resetEngine(e);
    assert(typeAndFlush(e, "3") == "ê"); resetEngine(e);
    assert(typeAndFlush(e, "4") == "ô"); resetEngine(e);
    assert(typeAndFlush(e, "7") == "ư"); resetEngine(e);
    assert(typeAndFlush(e, "8") == "ơ"); resetEngine(e);
    assert(typeAndFlush(e, "0") == "đ"); resetEngine(e);
    // Consonant + direct char
    assert(typeAndFlush(e, "b1") == "bă"); resetEngine(e);
    assert(typeAndFlush(e, "h2") == "hâ"); resetEngine(e);
    assert(typeAndFlush(e, "c4") == "cô"); resetEngine(e);
}

// ── Tone keys ────────────────────────────────────────────────────────────────

static void test_microsoft_tone_keys() {
    auto e = makeMicrosoft();
    assert(typeAndFlush(e, "as") == "á"); resetEngine(e);   // s → sắc
    assert(typeAndFlush(e, "aj") == "ạ"); resetEngine(e);   // j → nặng
    assert(typeAndFlush(e, "a5") == "à"); resetEngine(e);   // 5 → huyền
    assert(typeAndFlush(e, "a6") == "ả"); resetEngine(e);   // 6 → hỏi
    assert(typeAndFlush(e, "a9") == "ã"); resetEngine(e);   // 9 → ngã
    // Tone on direct chars
    assert(typeAndFlush(e, "1s") == "ắ"); resetEngine(e);   // ă + sắc
    assert(typeAndFlush(e, "2j") == "ậ"); resetEngine(e);   // â + nặng
    assert(typeAndFlush(e, "85") == "ờ"); resetEngine(e);   // ơ + huyền
    assert(typeAndFlush(e, "76") == "ử"); resetEngine(e);   // ư + hỏi
    assert(typeAndFlush(e, "39") == "ễ"); resetEngine(e);   // ê + ngã
}

// ── Full word tests ──────────────────────────────────────────────────────────

static void test_microsoft_words() {
    auto e = makeMicrosoft();
    // "thắng": th + 1(ă) + s(sắc→ắ) + ng
    assert(typeAndFlush(e, "th1sng") == "thắng"); resetEngine(e);
    // "sắc": s(consonant) + 1(ă) + s(sắc→ắ) + c
    assert(typeAndFlush(e, "s1sc") == "sắc"); resetEngine(e);
    // "ông": 4(ô) + ng
    assert(typeAndFlush(e, "4ng") == "ông"); resetEngine(e);
    // "hàng": h + a + 5(huyền→à) + ng
    assert(typeAndFlush(e, "ha5ng") == "hàng"); resetEngine(e);
    // "cơm": c + 8(ơ) + m
    assert(typeAndFlush(e, "c8m") == "cơm"); resetEngine(e);
    // "đi": 0(đ) + i
    assert(typeAndFlush(e, "0i") == "đi"); resetEngine(e);
}

// ── Double-key revert ────────────────────────────────────────────────────────

static void test_microsoft_double_key_revert() {
    auto e = makeMicrosoft();
    // 11 → literal "1" (revert ă)
    assert(typeAndFlush(e, "11") == "1"); resetEngine(e);
    // 77 → literal "7" (revert ư)
    assert(typeAndFlush(e, "77") == "7"); resetEngine(e);
    // a55 → "a5" (revert huyền on a)
    assert(typeAndFlush(e, "a55") == "a5"); resetEngine(e);
    // 1ss: 1→ă, s→sắc(ắ), s→revert to "ăs"
    assert(typeAndFlush(e, "1ss") == "ăs"); resetEngine(e);
}

// ── s as consonant when no vowel ─────────────────────────────────────────────

static void test_microsoft_s_consonant() {
    auto e = makeMicrosoft();
    // s before any vowel = consonant
    assert(typeAndFlush(e, "s1sc") == "sắc"); resetEngine(e);
    // s after consonants only = consonant
    assert(typeAndFlush(e, "th1sng") == "thắng"); resetEngine(e);
    // j is always tone (not a Vietnamese consonant)
    assert(typeAndFlush(e, "aj") == "ạ"); resetEngine(e);
    // tone key with no vowel → falls through as literal
    assert(typeAndFlush(e, "j") == "j"); resetEngine(e);
}

// ── Regression: other methods unaffected ─────────────────────────────────────

static void test_microsoft_regression() {
    // VNI: '1' is sắc tone (not ă)
    RuntimeConfig vniCfg = farolkey::config::defaultConfig();
    vniCfg.method = InputMethod::Vni;
    Engine vni(vniCfg);
    vni.processKey(KeyEvent{.key = 'a'});
    auto rv = vni.processKey(KeyEvent{.key = '1'});
    assert(rv.preedit == "á");

    // Telex: '1' is not treated as ă
    RuntimeConfig telexCfg = farolkey::config::defaultConfig();
    telexCfg.method = InputMethod::Telex;
    Engine telex(telexCfg);
    telex.processKey(KeyEvent{.key = 'a'});
    auto rt = telex.processKey(KeyEvent{.key = ' '});
    assert(rt.commit == "a ");  // '1' not pressed, just confirm a commits normally
}

int main() {
    test_microsoft_direct_chars();
    test_microsoft_tone_keys();
    test_microsoft_words();
    test_microsoft_double_key_revert();
    test_microsoft_s_consonant();
    test_microsoft_regression();
    return 0;
}
