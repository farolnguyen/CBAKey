// M14.12 — Free Layout smoke tests
// Test layout: VNI tones (1-5), diacritics (6-9, 0), shortcuts f→ph / j→gi / w→ng
#include <cassert>
#include <string>
#include "farolkey/core/engine.h"
#include "farolkey/core/free_layout_config.h"
#include "farolkey/core/types.h"
#include "farolkey/config/config.h"

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::FreeLayoutConfig;
using farolkey::core::FreeLayoutRule;
using farolkey::config::RuntimeConfig;

static Engine makeFL() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::FreeLayout;
    cfg.freeLayout.tones.tone_sac      = '1';
    cfg.freeLayout.tones.tone_huyen    = '2';
    cfg.freeLayout.tones.tone_hoi      = '3';
    cfg.freeLayout.tones.tone_nga      = '4';
    cfg.freeLayout.tones.tone_nang     = '5';
    cfg.freeLayout.tones.diacritic_mui   = '6';
    cfg.freeLayout.tones.diacritic_breve = '8';
    cfg.freeLayout.tones.diacritic_moc   = '7';
    cfg.freeLayout.tones.diacritic_d     = '9';
    cfg.freeLayout.tones.remove          = '0';
    cfg.freeLayout.shortcuts = {
        {'f', "ph"}, {'j', "gi"}, {'w', "ng"},
    };
    return Engine(cfg);
}

static std::string typeAndFlush(Engine& e, const std::string& seq) {
    std::string result;
    for (char c : seq) {
        auto r = e.processKey({.key = c});
        result += r.commit;
    }
    auto r = e.processKey({.key = ' '});
    result += r.commit;
    if (!result.empty() && result.back() == ' ')
        result = result.substr(0, result.size() - 1);
    return result;
}

static void reset(Engine& e) { e.clearState(); }

// ── Tone keys ────────────────────────────────────────────────────────────────

static void test_tones() {
    auto e = makeFL();
    assert(typeAndFlush(e, "a1") == "á"); reset(e);
    assert(typeAndFlush(e, "a2") == "à"); reset(e);
    assert(typeAndFlush(e, "a3") == "ả"); reset(e);
    assert(typeAndFlush(e, "a4") == "ã"); reset(e);
    assert(typeAndFlush(e, "a5") == "ạ"); reset(e);
    assert(typeAndFlush(e, "a11") == "a1"); reset(e);  // double-key revert
}

// ── Diacritic keys ───────────────────────────────────────────────────────────

static void test_diacritics() {
    auto e = makeFL();
    assert(typeAndFlush(e, "a6") == "â"); reset(e);
    assert(typeAndFlush(e, "a8") == "ă"); reset(e);
    assert(typeAndFlush(e, "e6") == "ê"); reset(e);
    assert(typeAndFlush(e, "o6") == "ô"); reset(e);
    assert(typeAndFlush(e, "o7") == "ơ"); reset(e);
    assert(typeAndFlush(e, "u7") == "ư"); reset(e);
}

// ── D-stroke ─────────────────────────────────────────────────────────────────

static void test_d_stroke() {
    auto e = makeFL();
    assert(typeAndFlush(e, "da9") == "đa"); reset(e);  // d in buffer → đ via '9'
    assert(typeAndFlush(e, "di") == "di");  reset(e);  // literal 'd' when no '9'
}

// ── Remove diacritics ─────────────────────────────────────────────────────────

static void test_remove() {
    auto e = makeFL();
    assert(typeAndFlush(e, "a10") == "a"); reset(e);
    assert(typeAndFlush(e, "a61") == "ấ"); reset(e);  // â + sắc = ấ
    assert(typeAndFlush(e, "a610") == "a"); reset(e); // â+sắc → remove all → a
}

// ── Shortcut keys (initial — no vowel) ───────────────────────────────────────

static void test_shortcuts() {
    auto e = makeFL();
    assert(typeAndFlush(e, "fo") == "pho");  reset(e);  // f → ph
    assert(typeAndFlush(e, "ja") == "gia");  reset(e);  // j → gi
    assert(typeAndFlush(e, "wa") == "nga");  reset(e);  // w → ng
    // Unmapped key = literal
    assert(typeAndFlush(e, "ba") == "ba");   reset(e);
}

// ── Context: shortcut key used as tone after vowel ───────────────────────────

static void test_conflict_context() {
    auto e = makeFL();
    // '1' is tone_sac; '1' after 'a' → sắc, not literal
    assert(typeAndFlush(e, "a1") == "á");    reset(e);
    // shortcut 'f' → ph (no vowel); 'f' after vowel 'o' → no tone assigned to 'f' → literal
    assert(typeAndFlush(e, "of") == "of");   reset(e);
}

// ── Full words ────────────────────────────────────────────────────────────────

static void test_words() {
    auto e = makeFL();
    assert(typeAndFlush(e, "fo73") == "phở");  reset(e);  // f→ph, o7→ơ, 3→hỏi
    assert(typeAndFlush(e, "ja2")  == "già");   reset(e);  // j→gi, a, 2→huyền
    assert(typeAndFlush(e, "wang") == "ngang"); reset(e);  // w→ng, a, n, g literal
    assert(typeAndFlush(e, "a6")   == "â");     reset(e);  // diacritic mũ
}

// ── Regression: other methods unaffected ─────────────────────────────────────

static void test_regression() {
    RuntimeConfig vniCfg = farolkey::config::defaultConfig();
    vniCfg.method = InputMethod::Vni;
    Engine vni(vniCfg);
    vni.processKey({.key = 'a'});
    auto rv = vni.processKey({.key = '1'});
    assert(rv.preedit == "á");  // VNI: 1 is still sắc
}

int main() {
    test_tones();
    test_diacritics();
    test_d_stroke();
    test_remove();
    test_shortcuts();
    test_conflict_context();
    test_words();
    test_regression();
    return 0;
}
