// M14.7 — Simple Telex smoke tests
// Simple Telex: only 5 tone keys active (s/f/r/x/j).
// All diacritic/transform keys (a/e/o/w/d/z) are literal.
// Double-key revert works for tone keys (e.g. baf+f -> baf literal).
#include <cassert>
#include <string>
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"
#include "farolkey/config/config.h"

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::config::RuntimeConfig;

static Engine makeSimpleTelex() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::SimpleTelex;
    return Engine(cfg);
}

// Accumulates all commits across the sequence, then strips trailing space.
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

// ── Tones work ───────────────────────────────────────────────────────────────

static void test_simple_telex_tones() {
    auto e = makeSimpleTelex();
    // s → sắc
    assert(typeAndFlush(e, "as") == "á"); resetEngine(e);
    // f → huyền
    assert(typeAndFlush(e, "af") == "à"); resetEngine(e);
    // r → hỏi
    assert(typeAndFlush(e, "ar") == "ả"); resetEngine(e);
    // x → ngã
    assert(typeAndFlush(e, "ax") == "ã"); resetEngine(e);
    // j → nặng
    assert(typeAndFlush(e, "aj") == "ạ"); resetEngine(e);
    // tones on words
    assert(typeAndFlush(e, "bans") == "bán"); resetEngine(e);
    assert(typeAndFlush(e, "banf") == "bàn"); resetEngine(e);
}

// ── Diacritic keys are literal ───────────────────────────────────────────────

static void test_simple_telex_no_transforms() {
    auto e = makeSimpleTelex();
    // aa → aa (no circumflex — Telex aa→â not active)
    assert(typeAndFlush(e, "aa") == "aa"); resetEngine(e);
    // aw → aw (no ơ)
    assert(typeAndFlush(e, "aw") == "aw"); resetEngine(e);
    // ow → ow (no ơ)
    assert(typeAndFlush(e, "ow") == "ow"); resetEngine(e);
    // uw → uw (no ư)
    assert(typeAndFlush(e, "uw") == "uw"); resetEngine(e);
    // dd → dd (no đ)
    assert(typeAndFlush(e, "dd") == "dd"); resetEngine(e);
    // ee → ee
    assert(typeAndFlush(e, "ee") == "ee"); resetEngine(e);
    // oo → oo
    assert(typeAndFlush(e, "oo") == "oo"); resetEngine(e);
    // z → z literal (no remove-diacritics)
    assert(typeAndFlush(e, "az") == "az"); resetEngine(e);
}

// ── Double-key revert for tone keys ─────────────────────────────────────────

static void test_simple_telex_double_key_revert() {
    auto e = makeSimpleTelex();
    // baf+f → baf  (f applies huyền, second f reverts)
    assert(typeAndFlush(e, "baff") == "baf"); resetEngine(e);
    // ass → as  (s applies sắc, second s reverts)
    assert(typeAndFlush(e, "ass") == "as"); resetEngine(e);
    // arr → ar
    assert(typeAndFlush(e, "arr") == "ar"); resetEngine(e);
    // axx → ax
    assert(typeAndFlush(e, "axx") == "ax"); resetEngine(e);
    // ajj → aj
    assert(typeAndFlush(e, "ajj") == "aj"); resetEngine(e);
}

// ── Tones on base vowels only — no diacritic interference ──────────────────

static void test_simple_telex_tone_on_plain_vowels() {
    auto e = makeSimpleTelex();
    // Simple Telex can still put tones on base vowels
    // but NOT on transformed vowels (since transforms don't happen)
    // e.g. "tois" → "tóis"? No — tone placement follows syllable model
    // bans → bán (tone on a, n is coda)
    assert(typeAndFlush(e, "bans") == "bán"); resetEngine(e);
    // tois → tói (tone on o, i is coda — syllable model handles it)
    assert(typeAndFlush(e, "tois") == "tói"); resetEngine(e);
    // Simple Telex cannot produce diacritic vowels (ê/â/ô/ă/ơ/ư/đ)
    // but tones work correctly on base vowels:
    assert(typeAndFlush(e, "tois") == "tói"); resetEngine(e);
}

// ── Regression: Telex/VNI/VIQR/VIQR* unaffected ─────────────────────────────

static void test_simple_telex_0_regression() {
    // Telex: aa → â still works
    RuntimeConfig telexCfg = farolkey::config::defaultConfig();
    telexCfg.method = InputMethod::Telex;
    Engine telex(telexCfg);
    telex.processKey(KeyEvent{.key = 'a'});
    telex.processKey(KeyEvent{.key = 'a'});
    auto r = telex.processKey(KeyEvent{.key = ' '});
    assert(r.commit == "â ");

    // VIQR: o+ → ơ still works
    RuntimeConfig viqrCfg = farolkey::config::defaultConfig();
    viqrCfg.method = InputMethod::Viqr;
    Engine viqr(viqrCfg);
    for (char c : std::string("o+")) viqr.processKey(KeyEvent{.key = c});
    auto r2 = viqr.processKey(KeyEvent{.key = ' '});
    std::string s2 = r2.commit;
    if (!s2.empty() && s2.back() == ' ') s2.pop_back();
    assert(s2 == "ơ");

    // VIQR*: o* → ơ still works
    RuntimeConfig viqrStarCfg = farolkey::config::defaultConfig();
    viqrStarCfg.method = InputMethod::ViqrStar;
    Engine viqrStar(viqrStarCfg);
    for (char c : std::string("o*")) viqrStar.processKey(KeyEvent{.key = c});
    auto r3 = viqrStar.processKey(KeyEvent{.key = ' '});
    std::string s3 = r3.commit;
    if (!s3.empty() && s3.back() == ' ') s3.pop_back();
    assert(s3 == "ơ");
}

int main() {
    test_simple_telex_tones();
    test_simple_telex_no_transforms();
    test_simple_telex_double_key_revert();
    test_simple_telex_tone_on_plain_vowels();
    test_simple_telex_0_regression();
    return 0;
}
