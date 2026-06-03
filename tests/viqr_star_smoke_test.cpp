// M14.6 — VIQR* smoke tests
// VIQR* = VIQR with '*' instead of '+' for horn diacritics (ơ, ư).
// '+' is literal in VIQR*; '*' is a special key.
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

static Engine makeViqrStar() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::ViqrStar;
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

// ── applyViqrStarTransform unit tests ────────────────────────────────────────

static void test_viqr_star_transform_unit() {
    using farolkey::core::vi_syllable::applyViqrStarTransform;

    // * → ơ  (horn on o)
    { std::u32string b = U"o"; assert(applyViqrStarTransform(b, '*')); assert(b == U"ơ"); }
    // * → ư  (horn on u)
    { std::u32string b = U"u"; assert(applyViqrStarTransform(b, '*')); assert(b == U"ư"); }
    // * with tone preserved: ó* → ớ
    { std::u32string b = U"ó"; assert(applyViqrStarTransform(b, '*')); assert(b == U"ớ"); }
    // * with tone preserved: ụ* → ự
    { std::u32string b = U"ụ"; assert(applyViqrStarTransform(b, '*')); assert(b == U"ự"); }
    // + does NOT trigger horn in VIQR*
    { std::u32string b = U"o"; assert(!applyViqrStarTransform(b, '+')); assert(b == U"o"); }
    { std::u32string b = U"u"; assert(!applyViqrStarTransform(b, '+')); assert(b == U"u"); }
    // ^ → circumflex (identical to VIQR)
    { std::u32string b = U"a"; assert(applyViqrStarTransform(b, '^')); assert(b == U"â"); }
    { std::u32string b = U"e"; assert(applyViqrStarTransform(b, '^')); assert(b == U"ê"); }
    { std::u32string b = U"o"; assert(applyViqrStarTransform(b, '^')); assert(b == U"ô"); }
    // ( → breve (identical to VIQR)
    { std::u32string b = U"a"; assert(applyViqrStarTransform(b, '(')); assert(b == U"ă"); }
    // dd → đ (identical to VIQR)
    { std::u32string b = U"d"; assert(applyViqrStarTransform(b, 'd')); assert(b == U"đ"); }
    { std::u32string b = U"D"; assert(applyViqrStarTransform(b, 'd')); assert(b == U"Đ"); }
    // Invalid: a* → false (no horn for a)
    { std::u32string b = U"a"; assert(!applyViqrStarTransform(b, '*')); assert(b == U"a"); }
}

// ── Engine pipeline tests ────────────────────────────────────────────────────

static void test_viqr_star_horn_basic() {
    auto e = makeViqrStar();
    // o* → ơ
    assert(typeAndFlush(e, "o*") == "ơ"); resetEngine(e);
    // u* → ư
    assert(typeAndFlush(e, "u*") == "ư"); resetEngine(e);
    // o+ is literal in VIQR* (no horn triggered)
    assert(typeAndFlush(e, "o+") == "o+"); resetEngine(e);
    // u+ is literal in VIQR*
    assert(typeAndFlush(e, "u+") == "u+"); resetEngine(e);
}

static void test_viqr_star_horn_with_tone() {
    auto e = makeViqrStar();
    // o*' → ớ  (horn then sắc)
    assert(typeAndFlush(e, "o*'") == "ớ"); resetEngine(e);
    // o*` → ờ  (horn then huyền)
    assert(typeAndFlush(e, "o*`") == "ờ"); resetEngine(e);
    // o*. → ợ  (horn then nặng)
    assert(typeAndFlush(e, "o*.") == "ợ"); resetEngine(e);
    // u*~ → ữ  (horn then ngã)
    assert(typeAndFlush(e, "u*~") == "ữ"); resetEngine(e);
    // u*? → ử  (horn then hỏi)
    assert(typeAndFlush(e, "u*?") == "ử"); resetEngine(e);
}

static void test_viqr_star_tone_then_horn() {
    auto e = makeViqrStar();
    // o'* → ớ  (sắc first, horn second — M17 normalizes tone position)
    assert(typeAndFlush(e, "o'*") == "ớ"); resetEngine(e);
    // u.* → ự
    assert(typeAndFlush(e, "u.*") == "ự"); resetEngine(e);
    // o~* → ỡ
    assert(typeAndFlush(e, "o~*") == "ỡ"); resetEngine(e);
}

static void test_viqr_star_escape_double_key() {
    auto e = makeViqrStar();
    // o** → o* (first * → ơ, second * reverts)
    assert(typeAndFlush(e, "o**") == "o*"); resetEngine(e);
    // a^^ → a^ (same as VIQR)
    assert(typeAndFlush(e, "a^^") == "a^"); resetEngine(e);
    // ba'' → ba' (tone escape, same as VIQR)
    assert(typeAndFlush(e, "ba''") == "ba'"); resetEngine(e);
    // ddd → dd (same as VIQR)
    assert(typeAndFlush(e, "ddd") == "dd"); resetEngine(e);
    // a(( → a( (breve escape)
    assert(typeAndFlush(e, "a((") == "a("); resetEngine(e);
}

static void test_viqr_star_remove_diacritics() {
    auto e = makeViqrStar();
    // o*'\  → o (remove horn + tone)
    assert(typeAndFlush(e, "o*'\\") == "o"); resetEngine(e);
    // u*\ → u (remove horn)
    assert(typeAndFlush(e, "u*\\") == "u"); resetEngine(e);
    // a^\ → a (remove circumflex)
    assert(typeAndFlush(e, "a^\\") == "a"); resetEngine(e);
    // \ with empty preedit → pass-through
    { auto r = e.processKey(KeyEvent{.key = '\\'}); resetEngine(e);
      assert(r.commit == "\\" || r.preedit == "\\" || !r.consumed); }
}

static void test_viqr_star_multisyllable() {
    auto e = makeViqrStar();
    // ddu*o*`ng → đường  (dd→đ, u*→ư, o*→ơ, `→huyền, ng coda)
    assert(typeAndFlush(e, "ddu*o*`ng") == "đường"); resetEngine(e);
    // ngu*o*`i → người
    assert(typeAndFlush(e, "ngu*o*`i") == "người"); resetEngine(e);
    // vie^.t → việt (^ still works)
    assert(typeAndFlush(e, "vie^.t") == "việt"); resetEngine(e);
    // tru*o*`ng → trường
    assert(typeAndFlush(e, "tru*o*`ng") == "trường"); resetEngine(e);
}

static void test_viqr_star_uppercase() {
    auto e = makeViqrStar();
    // O*' → Ớ
    assert(typeAndFlush(e, "O*'") == "Ớ"); resetEngine(e);
    // U*. → Ự
    assert(typeAndFlush(e, "U*.") == "Ự"); resetEngine(e);
    // A^ → Â (unchanged from VIQR)
    assert(typeAndFlush(e, "A^") == "Â"); resetEngine(e);
}

static void test_viqr_star_0_regression() {
    // VIQR engine must not be affected by adding ViqrStar.
    RuntimeConfig viqrCfg = farolkey::config::defaultConfig();
    viqrCfg.method = InputMethod::Viqr;
    Engine viqr(viqrCfg);
    // o+ still works in VIQR
    for (char c : std::string("o+")) viqr.processKey(KeyEvent{.key = c});
    auto r = viqr.processKey(KeyEvent{.key = ' '});
    std::string committed = r.commit;
    if (!committed.empty() && committed.back() == ' ')
        committed = committed.substr(0, committed.size() - 1);
    assert(committed == "ơ");

    // Telex engine unaffected
    RuntimeConfig telexCfg = farolkey::config::defaultConfig();
    telexCfg.method = InputMethod::Telex;
    Engine telex(telexCfg);
    telex.processKey(KeyEvent{.key = 'a'});
    auto r2 = telex.processKey(KeyEvent{.key = 'f'});
    assert(r2.preedit == "à" || r2.commit == "à");
}

int main() {
    test_viqr_star_transform_unit();
    test_viqr_star_horn_basic();
    test_viqr_star_horn_with_tone();
    test_viqr_star_tone_then_horn();
    test_viqr_star_escape_double_key();
    test_viqr_star_remove_diacritics();
    test_viqr_star_multisyllable();
    test_viqr_star_uppercase();
    test_viqr_star_0_regression();
    return 0;
}
