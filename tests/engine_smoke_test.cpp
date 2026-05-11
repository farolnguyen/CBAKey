#include <cassert>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"

using cbakey::core::Engine;
using cbakey::core::InputMode;
using cbakey::core::KeyAux;
using cbakey::core::KeyEvent;
using cbakey::core::ProcessResult;

static std::string typeSequence(Engine& engine, const std::string& keys) {
    for (const char key : keys) {
        engine.processKey(KeyEvent{.key = key});
    }
    const auto commit = engine.processKey(KeyEvent{.key = ' '});
    return commit.commit;  // includes trailing space when committing via space
}

int main() {
    Engine engine(cbakey::config::defaultConfig());

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

    auto englishKey = engine.processKey(KeyEvent{.key = 'a'});
    assert(englishKey.consumed);
    assert(englishKey.commit == "a");

    engine.setInputMode(InputMode::Vietnamese);
    engine.clearState();

    // aa + s -> ấ
    assert(typeSequence(engine, "aas") == "ấ ");
    engine.clearState();

    // aw + f -> ằ
    assert(typeSequence(engine, "awf") == "ằ ");
    engine.clearState();

    // dd -> đ
    assert(typeSequence(engine, "dd") == "đ ");
    engine.clearState();

    // uw + j -> ự
    assert(typeSequence(engine, "uwj") == "ự ");
    engine.clearState();

    // Undo transformed keypresses using backspace history.
    engine.processKey(KeyEvent{.key = 'a'});
    engine.processKey(KeyEvent{.key = 'a'});  // â
    auto undo1 = engine.processKey(KeyEvent{.key = '\b'});
    assert(undo1.preedit == "a");
    auto undo2 = engine.processKey(KeyEvent{.key = '\b'});
    assert(undo2.preedit.empty());

    cbakey::config::RuntimeConfig vniConfig = cbakey::config::defaultConfig();
    vniConfig.method = cbakey::core::InputMethod::Vni;
    Engine vniEngine(vniConfig);

    // a6 + 1 -> ấ
    assert(typeSequence(vniEngine, "a61") == "ấ ");
    vniEngine.clearState();

    // a8 + 2 -> ằ
    assert(typeSequence(vniEngine, "a82") == "ằ ");
    vniEngine.clearState();

    // d9 -> đ
    assert(typeSequence(vniEngine, "d9") == "đ ");
    vniEngine.clearState();

    // u7 + 5 -> ự
    assert(typeSequence(vniEngine, "u75") == "ự ");
    vniEngine.clearState();

    vniEngine.processKey(KeyEvent{.key = 'a'});
    vniEngine.processKey(KeyEvent{.key = '6'});  // â
    auto vniUndo = vniEngine.processKey(KeyEvent{.key = '\b'});
    assert(vniUndo.preedit == "a");

    engine.setInputMode(InputMode::Vietnamese);
    engine.clearState();
    engine.processKey(KeyEvent{.key = 'a'});
    engine.processKey(KeyEvent{.key = 'a'});
    auto nav = engine.processKey(KeyEvent{.aux = KeyAux::Left});
    assert(nav.consumed && nav.forwardOriginalKey);
    assert(nav.commit == "â");

    engine.clearState();
    engine.processKey(KeyEvent{.key = 'x'});
    auto punct = engine.processKey(KeyEvent{.key = ','});
    assert(punct.consumed && punct.commit == "x,");

    engine.clearState();
    engine.processKey(KeyEvent{.key = 'a'});
    auto ent = engine.processKey(KeyEvent{.aux = KeyAux::Enter});
    assert(ent.commit == "a\n");

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
    for (const char k : {'t', 'u', '7', 'o', '7', 'i', '1'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "tưới");

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
    for (const char k : {'r', 'u', '7', 'o', '7', 'u', '5'}) {
        lastVni = vniEngine.processKey(KeyEvent{.key = k});
    }
    assert(lastVni.preedit == "rượu");

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

    return 0;
}
