// M14 — Simple Telex 2 smoke tests
// SimpleTelex2 = full Telex (a/e/o/w/d/z transforms + s/f/r/x/j tones)
// with one addition: 'w' standalone (no hookable vowel) → ư.
// This matches ibus-unikey's stelex2 (vne_telex_w fallback).
#include <cassert>
#include <string>
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"
#include "farolkey/config/config.h"

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::config::RuntimeConfig;

static Engine makeSimpleTelex2() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::SimpleTelex2;
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

// ── Defining feature: standalone 'w' → ư ────────────────────────────────────

static void test_simple_telex2_standalone_w() {
    auto e = makeSimpleTelex2();
    // w alone → ư
    assert(typeAndFlush(e, "w") == "ư"); resetEngine(e);
    // w + coda consonants → ư + coda
    assert(typeAndFlush(e, "wng") == "ưng"); resetEngine(e);
    assert(typeAndFlush(e, "wn") == "ưn"); resetEngine(e);
    assert(typeAndFlush(e, "wc") == "ưc"); resetEngine(e);
    // double-key revert: ww → w literal
    assert(typeAndFlush(e, "ww") == "w"); resetEngine(e);
    // w + tone → ứ etc.
    assert(typeAndFlush(e, "ws") == "ứ"); resetEngine(e);
    assert(typeAndFlush(e, "wf") == "ừ"); resetEngine(e);
}

// ── Standard Telex transforms still work ─────────────────────────────────────

static void test_simple_telex2_telex_transforms() {
    auto e = makeSimpleTelex2();
    // Diacritics
    assert(typeAndFlush(e, "aa") == "â"); resetEngine(e);
    assert(typeAndFlush(e, "aw") == "ă"); resetEngine(e);
    assert(typeAndFlush(e, "ow") == "ơ"); resetEngine(e);
    assert(typeAndFlush(e, "uw") == "ư"); resetEngine(e);
    assert(typeAndFlush(e, "ee") == "ê"); resetEngine(e);
    assert(typeAndFlush(e, "oo") == "ô"); resetEngine(e);
    assert(typeAndFlush(e, "dd") == "đ"); resetEngine(e);
    // Tones
    assert(typeAndFlush(e, "as") == "á"); resetEngine(e);
    assert(typeAndFlush(e, "af") == "à"); resetEngine(e);
    assert(typeAndFlush(e, "asz") == "a"); resetEngine(e);  // z removes tone (á→a)
    // Combined
    assert(typeAndFlush(e, "aws") == "ắ"); resetEngine(e);
    assert(typeAndFlush(e, "owf") == "ờ"); resetEngine(e);
    assert(typeAndFlush(e, "uwj") == "ự"); resetEngine(e);
}

// ── Full word tests ──────────────────────────────────────────────────────────

static void test_simple_telex2_words() {
    auto e = makeSimpleTelex2();
    // Words starting with ư via standalone w
    assert(typeAndFlush(e, "wng") == "ưng"); resetEngine(e);
    assert(typeAndFlush(e, "wngs") == "ứng"); resetEngine(e);
    // Standard Telex words (nặng = j, not '.')
    assert(typeAndFlush(e, "bans") == "bán"); resetEngine(e);
    assert(typeAndFlush(e, "vieejt") == "việt"); resetEngine(e);
    assert(typeAndFlush(e, "bawns") == "bắn"); resetEngine(e);
}

// ── Double-key revert still works for all Telex keys ────────────────────────

static void test_simple_telex2_double_key_revert() {
    auto e = makeSimpleTelex2();
    assert(typeAndFlush(e, "aaa") == "aa"); resetEngine(e);  // aa→â, third a reverts
    assert(typeAndFlush(e, "ww") == "w"); resetEngine(e);    // standalone w→ư reverts
    assert(typeAndFlush(e, "oww") == "ow"); resetEngine(e);  // ow→ơ reverts
    assert(typeAndFlush(e, "baff") == "baf"); resetEngine(e);
}

// ── Regression: other methods unaffected ─────────────────────────────────────

static void test_simple_telex2_0_regression() {
    // Telex: 'w' standalone is still literal in full Telex? No — Telex also
    // doesn't have standalone w in FarolKey. Let's just confirm aa→â works.
    RuntimeConfig telexCfg = farolkey::config::defaultConfig();
    telexCfg.method = InputMethod::Telex;
    Engine telex(telexCfg);
    telex.processKey(KeyEvent{.key = 'a'});
    telex.processKey(KeyEvent{.key = 'a'});
    auto r = telex.processKey(KeyEvent{.key = ' '});
    assert(r.commit == "â ");

    // SimpleTelex: 'w' is still literal (no standalone ư)
    RuntimeConfig stCfg = farolkey::config::defaultConfig();
    stCfg.method = InputMethod::SimpleTelex;
    Engine st(stCfg);
    st.processKey(KeyEvent{.key = 'w'});
    auto r2 = st.processKey(KeyEvent{.key = ' '});
    std::string s2 = r2.commit;
    if (!s2.empty() && s2.back() == ' ') s2.pop_back();
    // SimpleTelex: 'w' is pushed as literal (no transform)
    assert(s2 == "w");
}

int main() {
    test_simple_telex2_standalone_w();
    test_simple_telex2_telex_transforms();
    test_simple_telex2_words();
    test_simple_telex2_double_key_revert();
    test_simple_telex2_0_regression();
    return 0;
}
