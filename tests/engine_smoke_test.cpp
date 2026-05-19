#include <cassert>

#include "farolkey/config/config.h"
#include "farolkey/core/engine.h"

using farolkey::core::Engine;
using farolkey::core::InputMode;
using farolkey::core::KeyAux;
using farolkey::core::KeyEvent;
using farolkey::core::ProcessResult;

static std::string typeSequence(Engine& engine, const std::string& keys) {
    for (const char key : keys) {
        engine.processKey(KeyEvent{.key = key});
    }
    const auto commit = engine.processKey(KeyEvent{.key = ' '});
    return commit.commit;  // includes trailing space when committing via space
}

/// UTF-8 for "hoặc " (U+1EB7 = a with breve + dot below).
static std::string utf8HoacNangSpace() {
    std::string s = "ho";
    s.push_back(static_cast<char>(0xE1));
    s.push_back(static_cast<char>(0xBA));
    s.push_back(static_cast<char>(0xB7));
    s += "c ";
    return s;
}

int main() {
    Engine engine(farolkey::config::defaultConfig());

    auto result1 = engine.processKey(KeyEvent{.key = 'x'});
    assert(result1.consumed);
    assert(result1.preedit == "x");

    auto result2 = engine.processKey(KeyEvent{.key = ' '});
    assert(result2.consumed);
    assert(result2.commit == "x ");
    assert(result2.preedit.empty());

    auto toggle = engine.processKey(KeyEvent{.key = 'z', .ctrl = true, .alt = true});
    assert(toggle.consumed);
    assert(engine.inputMode() == InputMode::English);

    // EN mode now buffers chars as preedit (like VI mode) so abbreviations with
    // abbrev_mode="en" or "both" can be expanded on word boundary.
    auto englishKey = engine.processKey(KeyEvent{.key = 'a'});
    assert(englishKey.consumed);
    assert(englishKey.preedit == "a");   // buffered as preedit, not committed yet
    assert(englishKey.commit.empty());
    // Space flushes the buffer (no dict entry → commit "a ")
    auto englishSpace = engine.processKey(KeyEvent{.key = ' '});
    assert(englishSpace.consumed);
    assert(englishSpace.commit == "a ");
    assert(englishSpace.preedit.empty());

    engine.setInputMode(InputMode::Vietnamese);
    engine.clearState();

    // aa + s -> ấ
    assert(typeSequence(engine, "aas") == "ấ ");
    assert(typeSequence(engine, "aas") == std::string("\xE1\xBA\xA5 "));
    engine.clearState();

    // aw + f -> ằ
    assert(typeSequence(engine, "awf") == "ằ ");
    assert(typeSequence(engine, "awf") == std::string("\xE1\xBA\xB1 "));
    engine.clearState();

    // dd -> đ
    assert(typeSequence(engine, "dd") == "đ ");
    assert(typeSequence(engine, "dd") == std::string("\xC4\x91 "));
    engine.clearState();

    // uw + j -> ự
    assert(typeSequence(engine, "uwj") == "ự ");
    assert(typeSequence(engine, "uwj") == std::string("\xE1\xBB\xB1 "));
    engine.clearState();

    // Repeating the same Telex tone key should emit it literally.
    assert(typeSequence(engine, "herr") == "her ");
    engine.clearState();

    // Repeating the same Telex transform key should emit the literal pair.
    assert(typeSequence(engine, "buss") == "bus ");
    engine.clearState();
    assert(typeSequence(engine, "xooong") == "xoong ");
    engine.clearState();
    assert(typeSequence(engine, "aww") == "aw ");
    engine.clearState();
    assert(typeSequence(engine, "aaa") == "aa ");
    engine.clearState();
    assert(typeSequence(engine, "eee") == "ee ");
    engine.clearState();
    assert(typeSequence(engine, "ooo") == "oo ");
    engine.clearState();
    assert(typeSequence(engine, "oww") == "ow ");
    engine.clearState();
    assert(typeSequence(engine, "uww") == "uw ");
    engine.clearState();
    assert(typeSequence(engine, "ddd") == "dd ");
    engine.clearState();

    // If multiple different tone keys are pressed, the last one wins.
    assert(typeSequence(engine, "asf") == "à ");
    engine.clearState();

    // Telex z removes Vietnamese diacritics when there is something to erase.
    assert(typeSequence(engine, "awz") == "a ");
    engine.clearState();
    assert(typeSequence(engine, "aasz") == "a ");
    engine.clearState();
    assert(typeSequence(engine, "ddz") == "d ");
    engine.clearState();
    assert(typeSequence(engine, "uowz") == "uo ");
    engine.clearState();
    assert(typeSequence(engine, "thieeuz") == "thieu ");
    engine.clearState();
    // Uppercase onset + tone on medial i (same syllable as "giúp"); z must not peel onto bare "i".
    assert(typeSequence(engine, "Gisupz") == "Giup ");
    engine.clearState();
    assert(typeSequence(engine, "eez") == "e ");
    engine.clearState();
    assert(typeSequence(engine, "ooz") == "o ");
    engine.clearState();
    assert(typeSequence(engine, "owz") == "o ");
    engine.clearState();
    assert(typeSequence(engine, "uwz") == "u ");
    engine.clearState();

    // If there is nothing to erase, z stays literal.
    assert(typeSequence(engine, "az") == "az ");
    engine.clearState();

    // Telex backslash escapes special keys so they stay literal.
    assert(typeSequence(engine, "a\\w") == "aw ");
    engine.clearState();
    assert(typeSequence(engine, "a\\s") == "as ");
    engine.clearState();
    assert(typeSequence(engine, "a\\z") == "az ");
    engine.clearState();
    assert(typeSequence(engine, "a\\a") == "aa ");
    engine.clearState();
    assert(typeSequence(engine, "e\\e") == "ee ");
    engine.clearState();
    assert(typeSequence(engine, "o\\o") == "oo ");
    engine.clearState();
    assert(typeSequence(engine, "o\\w") == "ow ");
    engine.clearState();
    assert(typeSequence(engine, "u\\w") == "uw ");
    engine.clearState();
    assert(typeSequence(engine, "d\\d") == "dd ");
    engine.clearState();
    assert(typeSequence(engine, "a\\b") == "a\\b ");
    engine.clearState();
    assert(typeSequence(engine, "\\\\") == "\\ ");
    engine.clearState();

    // Common Telex interaction order should work both ways on transform keys.
    assert(typeSequence(engine, "asa") == "ấ ");
    engine.clearState();
    assert(typeSequence(engine, "ese") == "ế ");
    engine.clearState();
    assert(typeSequence(engine, "oso") == "ố ");
    engine.clearState();
    assert(typeSequence(engine, "asw") == "ắ ");
    engine.clearState();
    assert(typeSequence(engine, "usw") == "ứ ");
    engine.clearState();
    assert(typeSequence(engine, "awsf") == "ằ ");
    engine.clearState();
    assert(typeSequence(engine, "aasf") == "ầ ");
    engine.clearState();

    // `uo` should now support tone-before-transform ordering too.
    assert(typeSequence(engine, "thuosoc") == "thuốc ");
    engine.clearState();
    assert(typeSequence(engine, "huoswng") == "hướng ");
    engine.clearState();

    // hoặc: o + ă nucleus must match "oa" tone rules (regression: literal j / VNI 5).
    assert(typeSequence(engine, "hoawcj") == utf8HoacNangSpace());
    engine.clearState();

    // Backspace should delete the visible character, not roll back transform history.
    engine.processKey(KeyEvent{.key = 'a'});
    engine.processKey(KeyEvent{.key = 'a'});  // â
    auto undo1 = engine.processKey(KeyEvent{.key = '\b'});
    assert(undo1.preedit.empty());
    auto undo2 = engine.processKey(KeyEvent{.key = '\b'});
    assert(undo2.preedit.empty());

    farolkey::config::RuntimeConfig vniConfig = farolkey::config::defaultConfig();
    vniConfig.method = farolkey::core::InputMethod::Vni;
    Engine vniEngine(vniConfig);

    auto ctrlPaste = vniEngine.processKey(KeyEvent{.key = 'v', .ctrl = true});
    assert(!ctrlPaste.consumed);
    assert(ctrlPaste.commit.empty());

    // a6 + 1 -> ấ
    assert(typeSequence(vniEngine, "a61") == "ấ ");
    assert(typeSequence(vniEngine, "a61") == std::string("\xE1\xBA\xA5 "));
    vniEngine.clearState();

    // a8 + 2 -> ằ
    assert(typeSequence(vniEngine, "a82") == "ằ ");
    assert(typeSequence(vniEngine, "a82") == std::string("\xE1\xBA\xB1 "));
    vniEngine.clearState();

    // d9 -> đ
    assert(typeSequence(vniEngine, "d9") == "đ ");
    assert(typeSequence(vniEngine, "d9") == std::string("\xC4\x91 "));
    vniEngine.clearState();

    // u7 + 5 -> ự
    assert(typeSequence(vniEngine, "u75") == "ự ");
    assert(typeSequence(vniEngine, "u75") == std::string("\xE1\xBB\xB1 "));
    vniEngine.clearState();

    assert(typeSequence(vniEngine, "hoac85") == utf8HoacNangSpace());
    vniEngine.clearState();

    // chao + VNI 2 -> chào (tone 2 = huyền on the nucleus per VNI mapping).
    for (const char k : std::string("chao")) {
        vniEngine.processKey(KeyEvent{.key = k});
    }
    const auto chaoSac = vniEngine.processKey(KeyEvent{.key = '2'});
    assert(chaoSac.consumed);
    assert(chaoSac.preedit == std::string("ch\xC3\xA0o"));
    vniEngine.clearState();

    // Numpad digit while composing: commit current buffer, then forward the digit to the client.
    for (const char k : std::string("ab")) {
        vniEngine.processKey(KeyEvent{.key = k});
    }
    const auto padAfterAb = vniEngine.processKey(KeyEvent{.key = '1', .key_from_keypad = true});
    assert(padAfterAb.consumed);
    assert(padAfterAb.commit == "ab");
    assert(padAfterAb.forwardOriginalKey);
    vniEngine.clearState();

    // Repeating the same VNI tone key should emit it literally.
    assert(typeSequence(vniEngine, "a11") == "a1 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a22") == "a2 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "he33") == "he3 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a44") == "a4 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a55") == "a5 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a12") == "à ");
    vniEngine.clearState();

    // VNI transform keys 6/7/8/9 currently keep the first transform and append later digits.
    assert(typeSequence(vniEngine, "a66") == "â6 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "e66") == "ê6 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "o66") == "ô6 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a88") == "ă8 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "o77") == "ơ7 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "u77") == "ư7 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "d99") == "đ9 ");
    vniEngine.clearState();

    // VNI 0 removes Vietnamese diacritics when present.
    assert(typeSequence(vniEngine, "a10") == "a ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a60") == "a ");
    vniEngine.clearState();
    // Uppercase vowel + VNI tone / strip (tone tables are lowercase; uppercase must still match).
    assert(typeSequence(vniEngine, "A1") == std::string("\xC3\x81 "));
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "A10") == "A ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a160") == "a ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "d90") == "d ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "u70") == "u ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "u750") == "u ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "thue610") == "thue ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "thue160") == "thue ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "ruou7750") == "ruou ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a0") == "a0 ");
    vniEngine.clearState();

    // VNI backslash escapes digit keys so they stay literal.
    assert(typeSequence(vniEngine, "a\\1") == "a1 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a\\6") == "a6 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "u\\7") == "u7 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "d\\9") == "d9 ");
    vniEngine.clearState();
    assert(typeSequence(vniEngine, "a\\16") == "a16 ");
    vniEngine.clearState();

    vniEngine.processKey(KeyEvent{.key = 'a'});
    vniEngine.processKey(KeyEvent{.key = '6'});  // â
    auto vniUndo = vniEngine.processKey(KeyEvent{.key = '\b'});
    assert(vniUndo.preedit.empty());

    engine.setInputMode(InputMode::Vietnamese);
    engine.clearState();
    engine.processKey(KeyEvent{.key = 'a'});
    engine.processKey(KeyEvent{.key = 'a'});
    auto nav = engine.processKey(KeyEvent{.aux = KeyAux::Left});
    assert(nav.consumed && nav.forwardOriginalKey);
    assert(nav.commit == "â");

    engine.clearState();
    engine.processKey(KeyEvent{.key = 'b'});
    auto navUp = engine.processKey(KeyEvent{.aux = KeyAux::Up});
    assert(navUp.consumed && navUp.forwardOriginalKey);
    assert(navUp.commit == "b");

    engine.clearState();
    engine.processKey(KeyEvent{.key = 'x'});
    auto punct = engine.processKey(KeyEvent{.key = ','});
    assert(punct.consumed && punct.commit == "x,");

    engine.clearState();
    engine.processKey(KeyEvent{.key = 'a'});
    auto ent = engine.processKey(KeyEvent{.aux = KeyAux::Enter});
    assert(ent.commit == "a\n");

    engine.clearState();
    auto ctrlCopy = engine.processKey(KeyEvent{.key = 'c', .ctrl = true});
    assert(!ctrlCopy.consumed);
    assert(ctrlCopy.commit.empty());
    assert(ctrlCopy.preedit.empty());

    engine.clearState();
    engine.processKey(KeyEvent{.key = 'a'});
    ctrlCopy = engine.processKey(KeyEvent{.key = 'c', .ctrl = true});
    assert(!ctrlCopy.consumed);
    auto commitAfterShortcut = engine.processKey(KeyEvent{.key = ' '});
    assert(commitAfterShortcut.commit == "a ");

    engine.clearState();
    for (const char k : {'a', 'a', 's'}) {
        engine.processKey(KeyEvent{.key = k});
    }
    auto telexDelete = engine.processKey(KeyEvent{.aux = KeyAux::DeleteForward});
    assert(telexDelete.preedit.empty());

    // Tone on correct vowel: "chào" → grave on **a**, not **o**
    engine.clearState();
    ProcessResult lastTelex;
    for (const char k : {'c', 'h', 'a', 'o', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "chào");

    // M1 tone-placement path: practical `qu` should tone on `a`, not `u`.
    engine.clearState();
    for (const char k : {'q', 'u', 'a', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "quá");

    // M1 tone-placement path: practical `gi` should tone on the following vowel.
    engine.clearState();
    for (const char k : {'g', 'i', 'u', 'w', 'x'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "giữ");

    // M1 transform path: `uow` -> `ươ`
    engine.clearState();
    for (const char k : {'h', 'u', 'o', 'w', 'n', 'g'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hương");

    // M1 transform path: `uoo` -> `uô`
    engine.clearState();
    for (const char k : {'t', 'h', 'u', 'o', 'o', 's', 'c'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "thuốc");

    // M1 transform path: `iee` -> `iê`
    engine.clearState();
    for (const char k : {'t', 'i', 'e', 'e', 's', 'n', 'g'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "tiếng");

    // M1 tone-placement over `uya`: tone on `y`.
    engine.clearState();
    for (const char k : {'k', 'h', 'u', 'y', 'a', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "khuỷa");

    engine.clearState();
    for (const char k : {'q', 'u', 'o', 'o', 's', 'c'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "quốc");

    engine.clearState();
    for (const char k : {'t', 'h', 'u', 'y', 'e', 'e', 'f', 'n'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "thuyền");

    engine.clearState();
    for (const char k : {'n', 'g', 'h', 'i', 'e', 'e', 'n', 'g'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "nghiêng");

    engine.clearState();
    for (const char k : {'n', 'g', 'u', 'y', 'e', 'e', 'x', 'n'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "nguyễn");

    engine.clearState();
    for (const char k : {'g', 'i', 'o', 'i', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "giỏi");

    engine.clearState();
    for (const char k : {'n', 'u', 'o', 'w', 'c', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "nước");

    engine.clearState();
    for (const char k : {'h', 'u', 'o', 'w', 'n', 'g', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hướng");

    engine.clearState();
    for (const char k : {'g', 'i', 'a', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "già");

    engine.clearState();
    for (const char k : {'n', 'g', 'o', 'a', 'i', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "ngoài");

    engine.clearState();
    for (const char k : {'x', 'o', 'a', 'y', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "xoáy");

    engine.clearState();
    for (const char k : {'t', 'h', 'u', 'e', 'e', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "thuế");

    engine.clearState();
    for (const char k : {'t', 'u', 'o', 'o', 'i', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "tuổi");

    engine.clearState();
    for (const char k : {'c', 'u', 'o', 'w', 'i', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "cười");

    engine.clearState();
    for (const char k : {'g', 'a', 'a', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "gấu");

    engine.clearState();
    for (const char k : {'c', 'a', 'a', 'y', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "cấy");

    engine.clearState();
    for (const char k : {'n', 'e', 'e', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "nếu");

    engine.clearState();
    for (const char k : {'c', 'u', 'w', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "cứu");

    engine.clearState();
    for (const char k : {'h', 'i', 'e', 'e', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hiếu");

    engine.clearState();
    for (const char k : {'y', 'e', 'e', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "yếu");

    engine.clearState();
    for (const char k : {'m', 'u', 'o', 'o', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "muối");

    engine.clearState();
    for (const char k : {'t', 'u', 'o', 'w', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "tưới");

    engine.clearState();
    for (const char k : {'m', 'i', 'a', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "mía");

    engine.clearState();
    for (const char k : {'c', 'u', 'a', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "của");

    engine.clearState();
    for (const char k : {'h', 'u', 'w', 'a', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hứa");

    engine.clearState();
    for (const char k : {'t', 'h', 'u', 'y', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "thuỷ");

    engine.clearState();
    for (const char k : {'k', 'h', 'u', 'a', 'a', 'y', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "khuấy");

    engine.clearState();
    for (const char k : {'r', 'u', 'o', 'w', 'u', 'j'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "rượu");

    engine.clearState();
    for (const char k : {'t', 'h', 'u', 'o', 'w', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "thuở");

    engine.clearState();
    for (const char k : {'t', 'o', 'o', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "tối");

    engine.clearState();
    for (const char k : {'b', 'o', 'w', 'i', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "bởi");

    engine.clearState();
    for (const char k : {'t', 'u', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "túi");

    engine.clearState();
    for (const char k : {'g', 'u', 'w', 'i', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "gửi");

    engine.clearState();
    for (const char k : {'x', 'o', 'a', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "xóa");

    engine.clearState();
    for (const char k : {'k', 'h', 'o', 'e', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "khỏe");

    engine.clearState();
    for (const char k : {'t', 'o', 'a', 'n', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "toàn");

    engine.clearState();
    for (const char k : {'g', 'a', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "gái");

    engine.clearState();
    for (const char k : {'m', 'a', 'y', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "máy");

    engine.clearState();
    for (const char k : {'s', 'a', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "sáu");

    engine.clearState();
    for (const char k : {'k', 'h', 'e', 'o', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "khéo");

    engine.clearState();
    for (const char k : {'b', 'a', 'o', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "báo");

    engine.clearState();
    for (const char k : {'c', 'a', 'i', 'x'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "cãi");

    engine.clearState();
    for (const char k : {'h', 'a', 'y', 'x'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hãy");

    engine.clearState();
    for (const char k : {'c', 'h', 'a', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "cháu");

    engine.clearState();
    for (const char k : {'b', 'e', 'o', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "béo");

    engine.clearState();
    for (const char k : {'k', 'i', 'a', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "kìa");

    engine.clearState();
    for (const char k : {'x', 'o', 'e', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "xòe");

    engine.clearState();
    for (const char k : {'h', 'u', 'e', 'e', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "huế");

    engine.clearState();
    for (const char k : {'b', 'u', 'o', 'o', 'f', 'n'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "buồn");

    engine.clearState();
    for (const char k : {'b', 'u', 'w', 'a', 'x'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "bữa");

    engine.clearState();
    for (const char k : {'b', 'u', 'o', 'w', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "bướu");

    engine.clearState();
    for (const char k : {'h', 'u', 'y', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "huỷ");

    engine.clearState();
    for (const char k : {'l', 'a', 'a', 'u', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "lẩu");

    engine.clearState();
    for (const char k : {'b', 'a', 'a', 'y', 'x'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "bẫy");

    engine.clearState();
    for (const char k : {'d', 'd', 'e', 'e', 'u', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "đều");

    engine.clearState();
    for (const char k : {'n', 'o', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "nói");

    engine.clearState();
    for (const char k : {'c', 'o', 'o', 'i', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "cối");

    engine.clearState();
    for (const char k : {'m', 'o', 'w', 'i', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "mời");

    engine.clearState();
    for (const char k : {'b', 'u', 'i', 'j'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "bụi");

    engine.clearState();
    for (const char k : {'n', 'g', 'u', 'w', 'i', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "ngửi");

    engine.clearState();
    for (const char k : {'m', 'u', 'a', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "múa");

    engine.clearState();
    for (const char k : {'h', 'o', 'a', 'i', 'f'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hoài");

    engine.clearState();
    for (const char k : {'n', 'g', 'o', 'a', 'y', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "ngoáy");

    engine.clearState();
    for (const char k : {'h', 'u', 'w', 'u', 'x'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "hữu");

    engine.clearState();
    for (const char k : {'t', 'h', 'i', 'e', 'e', 'u', 's'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "thiếu");

    engine.clearState();
    for (const char k : {'y', 'e', 'e', 'u', 'r'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "yểu");

    engine.clearState();
    for (const char k : {'k', 'h', 'u', 'y', 'a'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "khuya");

    engine.clearState();
    for (const char k : {'h', 'u', 'a', 'a', 'y'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "huây");

    engine.clearState();
    for (const char k : {'h', 'u', 'o', 'w'}) {
        lastTelex = engine.processKey(KeyEvent{.key = k});
    }
    assert(lastTelex.preedit == "huơ");

    // VNI: hiện — ê + nặng on ê; digit 6 must not leak when tone applied before circumflex
    vniEngine.clearState();
    ProcessResult lastVni;
    for (const char k : {'h', 'i', 'e', '6', '5', 'n'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hiện");

    vniEngine.clearState();
    for (const char k : {'h', 'i', 'e', '2', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hiề");

    vniEngine.clearState();
    for (const char k : {'q', 'u', 'a', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "quá");

    vniEngine.clearState();
    for (const char k : {'h', 'u', '7', 'o', '7', 'n', 'g'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hương");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'o', '6', '1', 'c'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuốc");

    vniEngine.clearState();
    for (const char k : {'t', 'i', 'e', '6', '1', 'n', 'g'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tiếng");

    vniEngine.clearState();
    for (const char k : {'k', 'h', 'u', 'y', 'a', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "khuỷa");

    vniEngine.clearState();
    for (const char k : {'q', 'u', 'o', '6', '1', 'c'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "quốc");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'y', 'e', '6', '2', 'n'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuyền");

    vniEngine.clearState();
    for (const char k : {'n', 'g', 'h', 'i', 'e', '6', 'n', 'g'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "nghiêng");

    vniEngine.clearState();
    for (const char k : {'n', 'g', 'u', 'y', 'e', '6', '4', 'n'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "nguyễn");

    vniEngine.clearState();
    for (const char k : {'g', 'i', 'o', 'i', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "giỏi");

    vniEngine.clearState();
    for (const char k : {'n', 'u', '7', 'o', '7', '1', 'c'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "nước");

    vniEngine.clearState();
    for (const char k : {'h', 'u', '7', 'o', '7', '1', 'n', 'g'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hướng");

    vniEngine.clearState();
    for (const char k : {'h', 'u', '7', 'o', '1', '7', 'n', 'g'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hướng");

    vniEngine.clearState();
    for (const char k : {'g', 'i', 'a', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "già");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'o', '7', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuở");

    vniEngine.clearState();
    for (const char k : {'n', 'g', 'o', 'a', 'i', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "ngoài");

    vniEngine.clearState();
    for (const char k : {'x', 'o', 'a', 'y', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "xoáy");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'e', '6', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuế");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'e', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuế");

    vniEngine.clearState();
    for (const char k : {'t', 'u', 'o', '6', 'i', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tuổi");

    vniEngine.clearState();
    for (const char k : {'c', 'u', '7', 'o', '7', 'i', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cười");

    vniEngine.clearState();
    for (const char k : {'c', 'u', 'o', 'i', '7', '1', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cưới");

    vniEngine.clearState();
    for (const char k : {'g', 'a', '6', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "gấu");

    vniEngine.clearState();
    for (const char k : {'c', 'a', '6', 'y', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cấy");

    vniEngine.clearState();
    for (const char k : {'n', 'e', '6', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "nếu");

    vniEngine.clearState();
    for (const char k : {'c', 'u', '7', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cứu");

    vniEngine.clearState();
    for (const char k : {'c', 'u', 'u', '7', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cứu");

    vniEngine.clearState();
    for (const char k : {'c', 'u', 'u', '1', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cứu");

    vniEngine.clearState();
    for (const char k : {'h', 'i', 'e', '6', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hiếu");

    vniEngine.clearState();
    for (const char k : {'y', 'e', '6', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "yếu");

    vniEngine.clearState();
    for (const char k : {'m', 'u', 'o', '6', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "muối");

    vniEngine.clearState();
    for (const char k : {'m', 'u', 'o', 'i', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "muối");

    vniEngine.clearState();
    for (const char k : {'t', 'u', '7', 'o', '7', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tưới");

    vniEngine.clearState();
    for (const char k : {'t', 'u', 'o', 'i', '3', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tuổi");

    vniEngine.clearState();
    for (const char k : {'m', 'i', 'a', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "mía");

    vniEngine.clearState();
    for (const char k : {'c', 'u', 'a', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "của");

    vniEngine.clearState();
    for (const char k : {'h', 'u', '7', 'a', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hứa");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'y', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuỷ");

    vniEngine.clearState();
    for (const char k : {'k', 'h', 'u', 'a', '6', 'y', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "khuấy");

    vniEngine.clearState();
    for (const char k : {'k', 'h', 'u', 'a', 'y', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "khuấy");

    vniEngine.clearState();
    for (const char k : {'r', 'u', '7', 'o', '7', 'u', '5'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', '7', 'o', '5', '7', 'u'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', 'o', 'u', '7', '7', '5'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', 'o', 'u', '7', '5', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', 'o', 'u', '5', '7', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', 'o', '7', 'u', '7', '5'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', '7', 'o', 'u', '5', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', '7', 'o', 'u', '7', '5'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    vniEngine.clearState();
    for (const char k : {'r', 'u', 'o', '7', 'u', '5', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

    // Broader representative VNI alternate-order coverage beyond the first M3 hotspot families.
    vniEngine.clearState();
    for (const char k : {'q', 'u', 'o', 'c', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "quốc");

    vniEngine.clearState();
    for (const char k : {'t', 'i', 'e', 'n', 'g', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tiếng");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'u', 'y', 'e', 'n', '2', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thuyền");

    vniEngine.clearState();
    for (const char k : {'n', 'g', 'u', 'y', 'e', 'n', '4', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "nguyễn");

    vniEngine.clearState();
    for (const char k : {'h', 'i', 'e', 'u', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hiếu");

    vniEngine.clearState();
    for (const char k : {'y', 'e', 'u', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "yếu");

    vniEngine.clearState();
    for (const char k : {'d', '9', 'e', 'u', '2', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "đều");

    vniEngine.clearState();
    for (const char k : {'t', 'o', 'i', '1', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tối");

    vniEngine.clearState();
    for (const char k : {'b', 'o', 'i', '3', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bởi");

    vniEngine.clearState();
    for (const char k : {'g', 'u', 'i', '3', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "gửi");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'a', '1', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hứa");

    vniEngine.clearState();
    for (const char k : {'b', 'u', 'o', 'u', '1', '7', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bướu");

    vniEngine.clearState();
    for (const char k : {'t', 'o', '6', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tối");

    vniEngine.clearState();
    for (const char k : {'b', 'o', '7', 'i', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bởi");

    vniEngine.clearState();
    for (const char k : {'t', 'u', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "túi");

    vniEngine.clearState();
    for (const char k : {'g', 'u', '7', 'i', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "gửi");

    vniEngine.clearState();
    for (const char k : {'x', 'o', 'a', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "xóa");

    vniEngine.clearState();
    for (const char k : {'k', 'h', 'o', 'e', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "khỏe");

    vniEngine.clearState();
    for (const char k : {'t', 'o', 'a', 'n', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "toàn");

    vniEngine.clearState();
    for (const char k : {'g', 'a', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "gái");

    vniEngine.clearState();
    for (const char k : {'m', 'a', 'y', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "máy");

    vniEngine.clearState();
    for (const char k : {'s', 'a', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "sáu");

    vniEngine.clearState();
    for (const char k : {'k', 'h', 'e', 'o', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "khéo");

    vniEngine.clearState();
    for (const char k : {'b', 'a', 'o', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "báo");

    vniEngine.clearState();
    for (const char k : {'c', 'a', 'i', '4'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cãi");

    vniEngine.clearState();
    for (const char k : {'h', 'a', 'y', '4'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hãy");

    vniEngine.clearState();
    for (const char k : {'c', 'h', 'a', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cháu");

    vniEngine.clearState();
    for (const char k : {'b', 'e', 'o', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "béo");

    vniEngine.clearState();
    for (const char k : {'k', 'i', 'a', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "kìa");

    vniEngine.clearState();
    for (const char k : {'x', 'o', 'e', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "xòe");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'e', '6', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "huế");

    vniEngine.clearState();
    for (const char k : {'b', 'u', 'o', '6', '2', 'n'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "buồn");

    vniEngine.clearState();
    for (const char k : {'b', 'u', '7', 'a', '4'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bữa");

    vniEngine.clearState();
    for (const char k : {'b', 'u', '7', 'o', '7', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bướu");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'y', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "huỷ");

    vniEngine.clearState();
    for (const char k : {'l', 'a', '6', 'u', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "lẩu");

    vniEngine.clearState();
    for (const char k : {'b', 'a', '6', 'y', '4'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bẫy");

    vniEngine.clearState();
    for (const char k : {'d', '9', 'e', '6', 'u', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "đều");

    vniEngine.clearState();
    for (const char k : {'d', 'e', 'u', '9', '6', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "đều");

    vniEngine.clearState();
    for (const char k : {'d', 'e', 'u', '2', '9', '6'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "đều");

    vniEngine.clearState();
    for (const char k : {'n', 'o', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "nói");

    vniEngine.clearState();
    for (const char k : {'c', 'o', '6', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "cối");

    vniEngine.clearState();
    for (const char k : {'m', 'o', '7', 'i', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "mời");

    vniEngine.clearState();
    for (const char k : {'b', 'u', 'i', '5'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "bụi");

    vniEngine.clearState();
    for (const char k : {'n', 'g', 'u', '7', 'i', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "ngửi");

    vniEngine.clearState();
    for (const char k : {'m', 'u', 'a', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "múa");

    vniEngine.clearState();
    for (const char k : {'h', 'o', 'a', 'i', '2'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hoài");

    vniEngine.clearState();
    for (const char k : {'n', 'g', 'o', 'a', 'y', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "ngoáy");

    vniEngine.clearState();
    for (const char k : {'h', 'u', '7', 'u', '4'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hữu");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'u', '7', '4'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hữu");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'u', '4', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "hữu");

    vniEngine.clearState();
    for (const char k : {'t', 'h', 'i', 'e', '6', 'u', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "thiếu");

    vniEngine.clearState();
    for (const char k : {'y', 'e', '6', 'u', '3'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "yểu");

    vniEngine.clearState();
    for (const char k : {'k', 'h', 'u', 'y', 'a'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "khuya");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'a', '6', 'y'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "huây");

    vniEngine.clearState();
    for (const char k : {'h', 'u', 'o', '7'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "huơ");

    // ---- "ua" + coda + tone: tone must land on â/a (second vowel), not u ----
    // Telex: c-h-u-a-a-n (aa->â) then r (hỏi) -> "chuẩn"; z must strip whole syllable.
    {
        Engine telexEngine(farolkey::config::defaultConfig());
        ProcessResult r;
        for (const char k : {'c', 'h', 'u', 'a', 'a', 'n', 'r'}) {
            r = telexEngine.processKey(KeyEvent{.key = k});
        }
        assert(r.preedit == "chuẩn");
        assert(r.commit.empty());
        auto rZ = telexEngine.processKey(KeyEvent{.key = 'z'});
        assert(rZ.preedit == "chuan");
        assert(rZ.commit.empty());
    }

    // Telex: t-u-a-a-n (aa->â) then f (huyền) -> "tuần"; z must strip whole syllable.
    {
        Engine telexEngine(farolkey::config::defaultConfig());
        ProcessResult r;
        for (const char k : {'t', 'u', 'a', 'a', 'n', 'f'}) {
            r = telexEngine.processKey(KeyEvent{.key = k});
        }
        assert(r.preedit == "tuần");
        assert(r.commit.empty());
        auto rZ = telexEngine.processKey(KeyEvent{.key = 'z'});
        assert(rZ.preedit == "tuan");
        assert(rZ.commit.empty());
    }

    // VNI: x-u-a-6 (6->â) then n then 3 (hỏi) -> "xuẩn"
    {
        farolkey::config::RuntimeConfig vniCfg2 = farolkey::config::defaultConfig();
        vniCfg2.method = farolkey::core::InputMethod::Vni;
        Engine vniEngine2(vniCfg2);
        ProcessResult r;
        for (const char k : {'x', 'u', 'a', '6', 'n', '3'}) {
            r = vniEngine2.processKey(KeyEvent{.key = k});
        }
        assert(r.preedit == "xuẩn");
        assert(r.commit.empty());
        auto rZ = vniEngine2.processKey(KeyEvent{.key = '0'});
        assert(rZ.preedit == "xuan");
        assert(rZ.commit.empty());
    }

    // --- 'uo' diphthong: Telex 'w' + coda → 'ươ', VNI '7' + coda → 'ươ' ---
    // Bug: VNI only transformed 'o'→'ơ' but not 'u'→'ư', giving 'đuợc' instead of 'được'.
    // Fix: VNI now calls normalizeTelexBuffer after vowel transforms, same as Telex.
    {
        farolkey::config::RuntimeConfig uoTelexCfg = farolkey::config::defaultConfig();
        farolkey::config::RuntimeConfig uoVniCfg   = farolkey::config::defaultConfig();
        uoVniCfg.method = farolkey::core::InputMethod::Vni;

        // Telex: dduowcj + Space → "được "
        // Intermediate "đuơ" after 'w' is expected; 'c' triggers normalization to "đươc".
        Engine e1(uoTelexCfg);
        assert(typeSequence(e1, "dduowcj") == "\xC4\x91\xC6\xB0\xe1\xbb\xa3\x63 "); // "được "
        e1.clearState();

        // Telex: vuowjt + Space → "vượt "
        Engine e2(uoTelexCfg);
        assert(typeSequence(e2, "vuowjt") == "v\xC6\xB0\xe1\xbb\xa3t ");  // "vượt "
        e2.clearState();

        // Telex: truowjt + Space → "trượt "
        Engine e3(uoTelexCfg);
        assert(typeSequence(e3, "truowjt") == "tr\xC6\xB0\xe1\xbb\xa3t ");  // "trượt "
        e3.clearState();

        // Telex: thuowr + Space → "thuở " (u stays as u, NOT ư, because no coda)
        Engine e4(uoTelexCfg);
        assert(typeSequence(e4, "thuowr") == "thu\xe1\xbb\x9f ");  // "thuở "
        e4.clearState();

        // VNI: d9uoc75 + Space → "được " (7 normalizes uơ→ươ because coda present)
        Engine e5(uoVniCfg);
        assert(typeSequence(e5, "d9uoc75") == "\xC4\x91\xC6\xB0\xe1\xbb\xa3\x63 ");  // "được "
        e5.clearState();

        // VNI: vuot75 + Space → "vượt "
        Engine e6(uoVniCfg);
        assert(typeSequence(e6, "vuot75") == "v\xC6\xB0\xe1\xbb\xa3t ");  // "vượt "
        e6.clearState();

        // VNI: thuo73 + Space → "thuở " (u stays as u, no coda → no normalization)
        Engine e7(uoVniCfg);
        assert(typeSequence(e7, "thuo73") == "thu\xe1\xbb\x9f ");  // "thuở "
        e7.clearState();
    }

    // --- Uppercase Đ regression (bug: uppercase D + VNI 9 was producing "D9") ---
    {
        farolkey::config::RuntimeConfig uVniCfg = farolkey::config::defaultConfig();
        uVniCfg.method = farolkey::core::InputMethod::Vni;
        farolkey::config::RuntimeConfig uTelCfg = farolkey::config::defaultConfig();

        // VNI: D + 9 → preedit "Đ"
        Engine vniU(uVniCfg);
        auto rD = vniU.processKey(KeyEvent{.key = 'D'});
        assert(rD.preedit == "D");
        auto r9 = vniU.processKey(KeyEvent{.key = '9'});
        assert(r9.preedit == "\xC4\x90");  // U+0110 = Đ
        assert(r9.commit.empty());

        // VNI: D + 9 + space → commit "Đ "
        Engine vniU2(uVniCfg);
        assert(typeSequence(vniU2, "D9") == "\xC4\x90 ");  // "Đ "
        vniU2.clearState();

        // Telex: Shift+D then d → "Đ" (engine lowercases key to 'd')
        Engine telU(uTelCfg);
        auto rT1 = telU.processKey(KeyEvent{.key = 'D'});
        assert(rT1.preedit == "D");
        auto rT2 = telU.processKey(KeyEvent{.key = 'D'});  // key lowercased to 'd' by engine
        assert(rT2.preedit == "\xC4\x90");  // Đ
        assert(rT2.commit.empty());
    }

    // --- 'ươ'↔'uô' toggle bugs (Bug 1 + Bug 2) ---
    {
        farolkey::config::RuntimeConfig telCfg = farolkey::config::defaultConfig();
        farolkey::config::RuntimeConfig vniCfg_ = farolkey::config::defaultConfig();
        vniCfg_.method = farolkey::core::InputMethod::Vni;

        // Bug 1 fix: Telex 'ươ' + 'w' → revert to 'uo' (strip diacritics + tone)
        // User can then re-apply whatever transform they want.
        {
            Engine e(telCfg);
            // Type 'được' (dduowcj) then press 'w' to strip ươ → 'đuoc'
            e.processKey(KeyEvent{.key = 'd'});
            e.processKey(KeyEvent{.key = 'd'});  // → đ
            e.processKey(KeyEvent{.key = 'u'});
            e.processKey(KeyEvent{.key = 'o'});
            e.processKey(KeyEvent{.key = 'w'});  // ow → ơ → đuơ
            e.processKey(KeyEvent{.key = 'c'});  // normalize: đươc
            e.processKey(KeyEvent{.key = 'j'});  // nặng → được
            auto rW = e.processKey(KeyEvent{.key = 'w'});  // strip ươ → đuoc
            assert(rW.preedit == "\xC4\x91uoc");  // "đuoc"
        }

        // Bug 2 fix: VNI 'uô' + '7' → 'ươ' (transform BOTH, preserve tone)
        {
            Engine e(vniCfg_);
            e.processKey(KeyEvent{.key = 'd'});
            e.processKey(KeyEvent{.key = '9'});  // d9 → đ (VNI)
            e.processKey(KeyEvent{.key = 'u'});
            e.processKey(KeyEvent{.key = 'o'});
            e.processKey(KeyEvent{.key = 'c'});  // → đuoc
            e.processKey(KeyEvent{.key = '6'});  // o → ô → đuôc
            e.processKey(KeyEvent{.key = '1'});  // sắc → đuốc
            // Now press '7': uô → ươ (both transform, sắc preserved → ớ)
            auto r7 = e.processKey(KeyEvent{.key = '7'});  // → đướ c
            // ư = U+01B0 = 0xC6 0xB0, ớ = U+1EDB = 0xE1 0xBB 0x9B
            assert(r7.preedit == "\xC4\x91\xC6\xB0\xE1\xBB\x9B\x63");  // "đước"
        }

        // Bonus: Telex 'uô' + 'w' → 'ươ' (symmetric with VNI Bug 2 fix)
        {
            Engine e(telCfg);
            e.processKey(KeyEvent{.key = 'd'}); e.processKey(KeyEvent{.key = 'd'});
            e.processKey(KeyEvent{.key = 'u'});
            e.processKey(KeyEvent{.key = 'o'}); e.processKey(KeyEvent{.key = 'c'});
            // After 'c', normalizeTelexBuffer fires: đuoc → đuoc (nucleus "uo", no coda yet?)
            // Actually: 'đ' + 'u' + 'o' + 'c', nucleus "uo", coda "c".
            // normalizeTelexBuffer: tonePattern "uo" → startsWith "uơ"? NO. → no change.
            e.processKey(KeyEvent{.key = 'o'});  // o → ô → đuôc
            // Now press 'w': uô + 'w' → ươ (both transform, tone 0)
            auto rW = e.processKey(KeyEvent{.key = 'w'});
            // ư = 0xC6 0xB0, ơ = 0xC6 0xA1
            assert(rW.preedit == "\xC4\x91\xC6\xB0\xC6\xA1\x63");  // "đươc"
        }
    }

    return 0;
}
