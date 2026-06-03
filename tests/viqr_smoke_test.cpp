// M14.3 — VIQR smoke tests
// Verifies applyViqrTransform (diacritics + dd→đ) and the full engine
// pipeline with InputMethod::Viqr (tones, escape, remove-diacritics).
#include <cassert>
#include <string>
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"
#include "farolkey/core/vi_syllable.h"
#include "farolkey/config/config.h"

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::config::RuntimeConfig;

// ── Helpers ──────────────────────────────────────────────────────────────────

static Engine makeViqr() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::Viqr;
    return Engine(cfg);
}

// Type a sequence of chars; return commit+preedit of final state.
static std::string typeSeq(Engine& e, const std::string& seq) {
    std::string result;
    for (char c : seq) {
        auto r = e.processKey(KeyEvent{.key = c});
        result = r.commit + r.preedit;
    }
    return result;
}

// Type a sequence and flush with space; return committed text (without space).
static std::string typeAndFlush(Engine& e, const std::string& seq) {
    for (char c : seq)
        e.processKey(KeyEvent{.key = c});
    auto r = e.processKey(KeyEvent{.key = ' '});
    // commit includes the space suffix; strip it
    if (!r.commit.empty() && r.commit.back() == ' ')
        return r.commit.substr(0, r.commit.size() - 1);
    return r.commit;
}

static void resetEngine(Engine& e) { e.clearState(); }

// ── applyViqrTransform unit tests (diacritics) ────────────────────────────────

static void test_viqr_transform_diacritics() {
    // a^ → â
    { std::u32string b = U"a"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '^')); assert(b == U"â"); }
    // e^ → ê
    { std::u32string b = U"e"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '^')); assert(b == U"ê"); }
    // o^ → ô
    { std::u32string b = U"o"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '^')); assert(b == U"ô"); }
    // a( → ă
    { std::u32string b = U"a"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '(')); assert(b == U"ă"); }
    // o+ → ơ
    { std::u32string b = U"o"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '+')); assert(b == U"ơ"); }
    // u+ → ư
    { std::u32string b = U"u"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '+')); assert(b == U"ư"); }
    // dd → đ
    { std::u32string b = U"d"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, 'd')); assert(b == U"đ"); }
    // DD → Đ
    { std::u32string b = U"D"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, 'd')); assert(b == U"Đ"); }
    // ^ with tone preserved: á^ → ấ
    { std::u32string b = U"á"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '^')); assert(b == U"ấ"); }
    // ( with tone preserved: á( → ắ
    { std::u32string b = U"á"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '(')); assert(b == U"ắ"); }
    // + with tone preserved: ó+ → ớ
    { std::u32string b = U"ó"; assert(farolkey::core::vi_syllable::applyViqrTransform(b, '+')); assert(b == U"ớ"); }
    // Invalid: e( → false (no breve for e)
    { std::u32string b = U"e"; assert(!farolkey::core::vi_syllable::applyViqrTransform(b, '(')); assert(b == U"e"); }
    // Invalid: i^ → false (no circumflex for i in nucleus position alone)
    { std::u32string b = U"i"; assert(!farolkey::core::vi_syllable::applyViqrTransform(b, '^')); }
    // Invalid: a+ → false (no horn for a)
    { std::u32string b = U"a"; assert(!farolkey::core::vi_syllable::applyViqrTransform(b, '+')); }
    // d without preceding d → false
    { std::u32string b = U"a"; assert(!farolkey::core::vi_syllable::applyViqrTransform(b, 'd')); }
}

// ── Engine pipeline tests ────────────────────────────────────────────────────

static void test_viqr_basic_diacritics() {
    auto e = makeViqr();
    // a^ → â
    assert(typeAndFlush(e, "a^") == "â"); resetEngine(e);
    // e^ → ê
    assert(typeAndFlush(e, "e^") == "ê"); resetEngine(e);
    // o^ → ô
    assert(typeAndFlush(e, "o^") == "ô"); resetEngine(e);
    // a( → ă
    assert(typeAndFlush(e, "a(") == "ă"); resetEngine(e);
    // o+ → ơ
    assert(typeAndFlush(e, "o+") == "ơ"); resetEngine(e);
    // u+ → ư
    assert(typeAndFlush(e, "u+") == "ư"); resetEngine(e);
    // dd → đ
    assert(typeAndFlush(e, "dd") == "đ"); resetEngine(e);
}

static void test_viqr_tones_only() {
    auto e = makeViqr();
    // ba' → bá
    assert(typeAndFlush(e, "ba'") == "bá"); resetEngine(e);
    // ba` → bà
    assert(typeAndFlush(e, "ba`") == "bà"); resetEngine(e);
    // ba? → bả
    assert(typeAndFlush(e, "ba?") == "bả"); resetEngine(e);
    // ba~ → bã
    assert(typeAndFlush(e, "ba~") == "bã"); resetEngine(e);
    // ba. → bạ
    assert(typeAndFlush(e, "ba.") == "bạ"); resetEngine(e);
}

static void test_viqr_diacritic_then_tone() {
    auto e = makeViqr();
    // a^' → ấ
    assert(typeAndFlush(e, "a^'") == "ấ"); resetEngine(e);
    // a^` → ầ
    assert(typeAndFlush(e, "a^`") == "ầ"); resetEngine(e);
    // a(. → ặ
    assert(typeAndFlush(e, "a(.") == "ặ"); resetEngine(e);
    // o+' → ớ
    assert(typeAndFlush(e, "o+'") == "ớ"); resetEngine(e);
    // u+. → ự
    assert(typeAndFlush(e, "u+.") == "ự"); resetEngine(e);
    // o^~ → ỗ
    assert(typeAndFlush(e, "o^~") == "ỗ"); resetEngine(e);
}

static void test_viqr_tone_then_diacritic() {
    auto e = makeViqr();
    // a'^ → ấ  (tone first, diacritic second)
    assert(typeAndFlush(e, "a'^") == "ấ"); resetEngine(e);
    // a.( → ặ
    assert(typeAndFlush(e, "a.(") == "ặ"); resetEngine(e);
    // o'+ → ớ
    assert(typeAndFlush(e, "o'+") == "ớ"); resetEngine(e);
    // u.+ → ự
    assert(typeAndFlush(e, "u.+") == "ự"); resetEngine(e);
}

static void test_viqr_multisyllable() {
    auto e = makeViqr();
    // vie^.t → việt
    assert(typeAndFlush(e, "vie^.t") == "việt"); resetEngine(e);
    // ba'n → bán
    assert(typeAndFlush(e, "ba'n") == "bán"); resetEngine(e);
    // ddu+o+`ng → đường
    assert(typeAndFlush(e, "ddu+o+`ng") == "đường"); resetEngine(e);
    // hoa(.c → hoặc
    assert(typeAndFlush(e, "hoa(.c") == "hoặc"); resetEngine(e);
}

static void test_viqr_escape_double_key() {
    auto e = makeViqr();
    // Double-key escape requires a preceding vowel so the first key transforms.
    // ba'' → ba' (first ' applies sắc → bá, second ' reverts → ba')
    assert(typeAndFlush(e, "ba''") == "ba'"); resetEngine(e);
    // ba.. → ba. (first . applies nặng → bạ, second . reverts → ba.)
    assert(typeAndFlush(e, "ba..") == "ba."); resetEngine(e);
    // ban?? → ban? (first ? applies hỏi, second ? reverts)
    assert(typeAndFlush(e, "ban??") == "ban?"); resetEngine(e);
    // ba~~ → ba~
    assert(typeAndFlush(e, "ba~~") == "ba~"); resetEngine(e);
    // ba`` → ba`
    assert(typeAndFlush(e, "ba``") == "ba`"); resetEngine(e);
    // a^^ → a^ (first ^ → â, second ^ reverts → a^)
    assert(typeAndFlush(e, "a^^") == "a^"); resetEngine(e);
    // a(( → a( (first ( → ă, second ( reverts → a()
    assert(typeAndFlush(e, "a((") == "a("); resetEngine(e);
    // o++ → o+ (first + → ơ, second + reverts → o+)
    assert(typeAndFlush(e, "o++") == "o+"); resetEngine(e);
    // ddd → dd (dd→đ, third d reverts → dd)
    assert(typeAndFlush(e, "ddd") == "dd"); resetEngine(e);
}

static void test_viqr_remove_diacritics() {
    auto e = makeViqr();
    // ba^'\  → ba (remove diacritic + tone)
    assert(typeAndFlush(e, "ba^'\\") == "ba"); resetEngine(e);
    // a(\  → a (remove breve)
    assert(typeAndFlush(e, "a(\\") == "a"); resetEngine(e);
    // u+\ → u (remove horn)
    assert(typeAndFlush(e, "u+\\") == "u"); resetEngine(e);
    // \ with empty preedit → \ literal
    { auto r = e.processKey(KeyEvent{.key = '\\'}); resetEngine(e);
      // nothing to remove → pass-through as literal
      assert(r.commit == "\\" || r.preedit == "\\" || !r.consumed); }
}

static void test_viqr_uppercase() {
    auto e = makeViqr();
    // A^ → Â
    assert(typeAndFlush(e, "A^") == "Â"); resetEngine(e);
    // A^' → Ấ
    assert(typeAndFlush(e, "A^'") == "Ấ"); resetEngine(e);
    // O+' → Ớ
    assert(typeAndFlush(e, "O+'") == "Ớ"); resetEngine(e);
}

static void test_viqr_no_w_shortcut() {
    auto e = makeViqr();
    // ow → o + w literal (no Telex shortcut)
    auto r = e.processKey(KeyEvent{.key = 'o'});
    auto r2 = e.processKey(KeyEvent{.key = 'w'});
    // 'w' is not a VIQR key, pushed as literal
    std::string combined = r.commit + r.preedit + r2.commit + r2.preedit;
    assert(combined.find("ơ") == std::string::npos);
    resetEngine(e);
}

static void test_viqr_0_regression_telex_vni() {
    // Switching to Viqr must not break Telex/VNI engines in other instances.
    RuntimeConfig telexCfg = farolkey::config::defaultConfig();
    telexCfg.method = InputMethod::Telex;
    Engine telex(telexCfg);
    // 'af' → à (Telex tone still works)
    telex.processKey(KeyEvent{.key = 'a'});
    auto r = telex.processKey(KeyEvent{.key = 'f'});
    assert(r.preedit == "à" || r.commit == "à");

    RuntimeConfig vniCfg = farolkey::config::defaultConfig();
    vniCfg.method = InputMethod::Vni;
    Engine vni(vniCfg);
    vni.processKey(KeyEvent{.key = 'a'});
    auto r2 = vni.processKey(KeyEvent{.key = '1'});
    assert(r2.preedit == "á" || r2.commit == "á");
}

int main() {
    test_viqr_transform_diacritics();
    test_viqr_basic_diacritics();
    test_viqr_tones_only();
    test_viqr_diacritic_then_tone();
    test_viqr_tone_then_diacritic();
    test_viqr_multisyllable();
    test_viqr_escape_double_key();
    test_viqr_remove_diacritics();
    test_viqr_uppercase();
    test_viqr_no_w_shortcut();
    test_viqr_0_regression_telex_vni();
    return 0;
}
