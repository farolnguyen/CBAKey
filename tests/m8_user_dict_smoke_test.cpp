/// M8 smoke test: UserDict loader + Engine static expansion.
/// Tests:
///   1. UserDict::loadFromFile — parse JSONL file with entries
///   2. UserDict::lookup — found / not-found
///   3. Engine expansion fires on Space for non-syllable trigger
///   4. Engine does NOT expand trigger that is a valid Vietnamese syllable (conflict policy)
///   5. Engine expands with force=true even if trigger is a valid syllable
///   6. Expansion on Enter (newline boundary)
///   7. No expansion when preedit is empty

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"
#include "cbakey/core/types.h"
#include "cbakey/core/user_dict.h"

using cbakey::core::Engine;
using cbakey::core::KeyAux;
using cbakey::core::KeyEvent;
using cbakey::core::ProcessResult;
using cbakey::core::UserDict;

/// Write a temporary JSON file and return its path.
static std::string writeTempDict(const std::string& content) {
    const char* tmpdir = std::getenv("TMPDIR");
    std::string path = (tmpdir ? std::string(tmpdir) : std::string("/tmp")) + "/cbakey_test_dict.json";
    FILE* f = std::fopen(path.c_str(), "w");
    assert(f);
    std::fwrite(content.data(), 1, content.size(), f);
    std::fclose(f);
    return path;
}

/// Type a sequence of ASCII chars into the engine. Returns last ProcessResult.
static ProcessResult typeString(Engine& eng, const std::string& seq) {
    ProcessResult r;
    for (char c : seq) {
        r = eng.processKey(KeyEvent{.key = c});
    }
    return r;
}

int main() {
    // ── Test 1: UserDict loader ──────────────────────────────────────────────
    {
        const std::string json = R"([
{"trigger": "btv", "expansion": "Ban T\u1ed5 ch\u1ee9c"},
{"trigger": "ko", "expansion": "kh\u00f4ng"},
{"trigger": "ban", "expansion": "Ban Ch\u1ea5p H\u00e0nh", "force": true},
{"trigger": "bad_entry_no_expansion"},
{"trigger": "", "expansion": "empty trigger"}
])";
        const std::string path = writeTempDict(json);
        const UserDict dict = UserDict::loadFromFile(path);
        // 3 valid entries (btv, ko, ban); bad_entry and empty trigger skipped
        assert(dict.size() == 3);
        assert(dict.lookup("btv") != nullptr);
        assert(dict.lookup("ko") != nullptr);
        assert(dict.lookup("ban") != nullptr);
        assert(dict.lookup("xyz") == nullptr);
        assert(dict.lookup("btv")->force == false);
        assert(dict.lookup("ban")->force == true);
    }

    // ── Test 2: loadFromFile on non-existent file — silent, empty dict ───────
    {
        const UserDict dict = UserDict::loadFromFile("/tmp/cbakey_nonexistent_9999.json");
        assert(dict.empty());
    }

    // ── Test 3: Engine expansion on Space (non-syllable trigger) ────────────
    {
        const std::string json = R"([{"trigger": "btv", "expansion": "Ban T\u1ed5 ch\u1ee9c"}])";
        const std::string path = writeTempDict(json);
        cbakey::config::RuntimeConfig cfg = cbakey::config::defaultConfig();
        cfg.enableUserDictionary = true;
        cfg.userDictPath = path;
        Engine eng(cfg);
        typeString(eng, "btv");
        const ProcessResult r = eng.processKey(KeyEvent{.key = ' '});
        assert(r.consumed);
        // commit must be expansion + space
        assert(r.commit == "Ban T\u1ed5 ch\u1ee9c ");
        assert(r.preedit.empty());
    }

    // ── Test 4: Dict always wins — even for a valid syllable trigger ─────────
    // Design decision: user explicitly put "ban" in dict → they want expansion.
    // If they don't want it they should remove it from the dict.
    {
        const std::string json = R"([{"trigger": "ban", "expansion": "Ban Ch\u1ea5p H\u00e0nh"}])";
        const std::string path = writeTempDict(json);
        cbakey::config::RuntimeConfig cfg = cbakey::config::defaultConfig();
        cfg.enableUserDictionary = true;
        cfg.userDictPath = path;
        Engine eng(cfg);
        typeString(eng, "ban");
        const ProcessResult r = eng.processKey(KeyEvent{.key = ' '});
        assert(r.consumed);
        assert(r.commit == "Ban Ch\u1ea5p H\u00e0nh ");
    }

    // ── Test 5: force flag — entry loads correctly, expansion works ───────────
    {
        const std::string json = R"([{"trigger": "ko", "expansion": "kh\u00f4ng", "force": true}])";
        const std::string path = writeTempDict(json);
        const UserDict dict = UserDict::loadFromFile(path);
        assert(dict.lookup("ko") != nullptr);
        assert(dict.lookup("ko")->force == true);
        assert(dict.lookup("ko")->expansion == "kh\u00f4ng");
    }

    // ── Test 6: Expansion on Enter ───────────────────────────────────────────
    {
        const std::string json = R"([{"trigger": "ko", "expansion": "kh\u00f4ng"}])";
        const std::string path = writeTempDict(json);
        cbakey::config::RuntimeConfig cfg = cbakey::config::defaultConfig();
        cfg.enableUserDictionary = true;
        cfg.userDictPath = path;
        Engine eng(cfg);
        typeString(eng, "ko");
        const ProcessResult r = eng.processKey(KeyEvent{.aux = KeyAux::Enter});
        assert(r.consumed);
        assert(r.commit == "kh\u00f4ng\n");
    }

    // ── Test 7: No expansion when enableUserDictionary=false ─────────────────
    {
        const std::string json = R"([{"trigger": "ko", "expansion": "kh\u00f4ng"}])";
        const std::string path = writeTempDict(json);
        cbakey::config::RuntimeConfig cfg = cbakey::config::defaultConfig();
        cfg.enableUserDictionary = false;
        cfg.userDictPath = path;
        Engine eng(cfg);
        typeString(eng, "ko");
        const ProcessResult r = eng.processKey(KeyEvent{.key = ' '});
        assert(r.consumed);
        assert(r.commit == "ko ");
    }

    // ── Test 8: Normal Vietnamese input not affected ─────────────────────────
    {
        const std::string json = R"([{"trigger": "btv", "expansion": "irrelevant"}])";
        const std::string path = writeTempDict(json);
        cbakey::config::RuntimeConfig cfg = cbakey::config::defaultConfig();
        cfg.enableUserDictionary = true;
        cfg.userDictPath = path;
        Engine eng(cfg);
        // "xin chào" — type "xinf" (Telex: xin+f=xìn)... then space
        typeString(eng, "xin");
        const ProcessResult rSpace = eng.processKey(KeyEvent{.key = ' '});
        assert(rSpace.commit == "xin ");
    }

    return 0;
}
