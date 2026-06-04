// M14.11 — Tốc ký input method smoke tests
// Base VNI (tones 1-5, diacritics a6/a8/o7/u7/e6/o6, 0=xóa dấu)
// Initial shortcuts (no vowel): f→ph, j→gi, k→kh, c→k, z→d, d→đ, w→ng, q→qu
// Final shortcuts (after vowel): g→ng, h→nh, k→ch
#include <cassert>
#include <string>
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"
#include "farolkey/config/config.h"

using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::config::RuntimeConfig;

static Engine makeTocKy() {
    RuntimeConfig cfg = farolkey::config::defaultConfig();
    cfg.method = InputMethod::TocKy;
    return Engine(cfg);
}

// Flush entire sequence + space to trigger commit; strip trailing space.
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

// ── VNI base tones ────────────────────────────────────────────────────────────

static void test_toc_ky_vni_tones() {
    auto e = makeTocKy();
    assert(typeAndFlush(e, "a1") == "á");  resetEngine(e);  // sắc
    assert(typeAndFlush(e, "a2") == "à");  resetEngine(e);  // huyền
    assert(typeAndFlush(e, "a3") == "ả");  resetEngine(e);  // hỏi
    assert(typeAndFlush(e, "a4") == "ã");  resetEngine(e);  // ngã
    assert(typeAndFlush(e, "a5") == "ạ");  resetEngine(e);  // nặng
    assert(typeAndFlush(e, "a10") == "a"); resetEngine(e);  // sắc rồi xóa dấu
    assert(typeAndFlush(e, "a11") == "a1"); resetEngine(e); // double-key revert
}

// ── VNI base diacritics ───────────────────────────────────────────────────────

static void test_toc_ky_vni_diacritics() {
    auto e = makeTocKy();
    assert(typeAndFlush(e, "a6") == "â");  resetEngine(e);
    assert(typeAndFlush(e, "a8") == "ă");  resetEngine(e);
    assert(typeAndFlush(e, "e6") == "ê");  resetEngine(e);
    assert(typeAndFlush(e, "o6") == "ô");  resetEngine(e);
    assert(typeAndFlush(e, "o7") == "ơ");  resetEngine(e);
    assert(typeAndFlush(e, "u7") == "ư");  resetEngine(e);
}

// ── Initial consonant shortcuts ───────────────────────────────────────────────

static void test_toc_ky_initial_shortcuts() {
    auto e = makeTocKy();
    // f → ph
    assert(typeAndFlush(e, "fo") == "pho");  resetEngine(e);
    // j → gi
    assert(typeAndFlush(e, "ja") == "gia");  resetEngine(e);
    // k → kh (initial, no vowel)
    assert(typeAndFlush(e, "ki") == "khi");  resetEngine(e);
    // c → k
    assert(typeAndFlush(e, "ca") == "ka");   resetEngine(e);
    // z → d
    assert(typeAndFlush(e, "zo") == "do");   resetEngine(e);
    // d → đ
    assert(typeAndFlush(e, "di") == "đi");   resetEngine(e);
    // w → ng
    assert(typeAndFlush(e, "wa") == "nga");  resetEngine(e);
    // q → qu
    assert(typeAndFlush(e, "qa") == "qua");  resetEngine(e);
}

// ── Final consonant shortcuts ─────────────────────────────────────────────────

static void test_toc_ky_final_shortcuts() {
    auto e = makeTocKy();
    // g → ng (after vowel)
    assert(typeAndFlush(e, "ag") == "ang");  resetEngine(e);
    // h → nh (after vowel)
    assert(typeAndFlush(e, "ah") == "anh");  resetEngine(e);
    // k → ch (after vowel — disambiguation from k→kh initial)
    assert(typeAndFlush(e, "ak") == "ach");  resetEngine(e);
}

// ── k disambiguation: initial vs final ────────────────────────────────────────

static void test_toc_ky_k_disambiguation() {
    auto e = makeTocKy();
    // k without vowel = kh (initial)
    assert(typeAndFlush(e, "ki") == "khi");  resetEngine(e);
    assert(typeAndFlush(e, "koa") == "khoa"); resetEngine(e);
    // k after vowel = ch (final)
    assert(typeAndFlush(e, "ak") == "ach");  resetEngine(e);
    assert(typeAndFlush(e, "ik") == "ich");  resetEngine(e);
}

// ── Full words ────────────────────────────────────────────────────────────────

static void test_toc_ky_words() {
    auto e = makeTocKy();
    // "phở": f + o7 + 3 → ph + ơ + hỏi
    assert(typeAndFlush(e, "fo73") == "phở");    resetEngine(e);
    // "giờ": j + o7 + 2 → gi + ơ + huyền
    assert(typeAndFlush(e, "jo72") == "giờ");    resetEngine(e);
    // "khi": k + i → khi
    assert(typeAndFlush(e, "ki") == "khi");      resetEngine(e);
    // "được": d→đ, u+o+7→ươ, c, 5→nặng = "đươc5" → "được"
    assert(typeAndFlush(e, "duoc75") == "được"); resetEngine(e);
    // "ngang": w + a + g → ng + a + ng
    assert(typeAndFlush(e, "wag") == "ngang");   resetEngine(e);
    // "anh": a + h → a + nh
    assert(typeAndFlush(e, "ah") == "anh");      resetEngine(e);
    // c→k, a, h→nh, 5→nặng: output "kạnh" (c and k are same phoneme in Tốc ký)
    assert(typeAndFlush(e, "cah5") == "kạnh");   resetEngine(e);
    // c→k, a, k→ch, 5→nặng: output "kạch"
    assert(typeAndFlush(e, "cak5") == "kạch");   resetEngine(e);
    // "quê": q + e6 → qu + ê
    assert(typeAndFlush(e, "qe6") == "quê");     resetEngine(e);
    // "phím": f + i + m → ph + i + m (no tone)
    assert(typeAndFlush(e, "fim") == "phim");    resetEngine(e);
    // "già": j + a2 → gi + à (huyền)
    assert(typeAndFlush(e, "ja2") == "già");     resetEngine(e);
}

// ── Regression: other methods unaffected ─────────────────────────────────────

static void test_toc_ky_regression() {
    // VNI: 'f' is literal
    RuntimeConfig vniCfg = farolkey::config::defaultConfig();
    vniCfg.method = InputMethod::Vni;
    Engine vni(vniCfg);
    auto rv = vni.processKey(KeyEvent{.key = 'f'});
    assert(rv.preedit == "f");  // f is literal in VNI

    // Telex: 'j' is nặng tone
    RuntimeConfig telexCfg = farolkey::config::defaultConfig();
    telexCfg.method = InputMethod::Telex;
    Engine telex(telexCfg);
    telex.processKey(KeyEvent{.key = 'a'});
    auto rt = telex.processKey(KeyEvent{.key = 'j'});
    assert(rt.preedit == "ạ");  // j → nặng in Telex
}

int main() {
    test_toc_ky_vni_tones();
    test_toc_ky_vni_diacritics();
    test_toc_ky_initial_shortcuts();
    test_toc_ky_final_shortcuts();
    test_toc_ky_k_disambiguation();
    test_toc_ky_words();
    test_toc_ky_regression();
    return 0;
}
