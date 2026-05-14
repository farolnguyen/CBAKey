#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"
#include "cbakey/core/user_dict.h"

namespace fs = std::filesystem;

// ── Helper ──────────────────────────────────────────────────────────────────

static std::string runSequence(cbakey::core::Engine& eng, const std::string& seq) {
    std::string commits;
    for (char c : seq) {
        auto r = eng.processKey({.key = c});
        commits += r.commit;
    }
    return commits;
}

static std::string tmpDictPath() {
    return (fs::temp_directory_path() / "cbakey_m13_test_dict.json").string();
}

static void writeDictFile(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
}

// ── Schema / loader ──────────────────────────────────────────────────────────

TEST(M13Schema, LoadAbbrevMode) {
    const std::string p = tmpDictPath();
    writeDictFile(p,
        "{\"trigger\":\"ko\",\"expansion\":\"không\",\"abbrev_mode\":\"vi\"}\n"
        "{\"trigger\":\"doex\",\"expansion\":\"docker exec -ti\",\"abbrev_mode\":\"en\"}\n"
        "{\"trigger\":\"btv\",\"expansion\":\"Ban Tổ chức\",\"abbrev_mode\":\"both\"}\n"
        "{\"trigger\":\"old\",\"expansion\":\"legacy\"}\n");  // no abbrev_mode → defaults Vi
    const auto d = cbakey::core::UserDict::loadFromFile(p);
    ASSERT_EQ(d.size(), 4u);

    EXPECT_EQ(d.lookup("ko")->abbrev_mode,   cbakey::core::AbbrevMode::Vi);
    EXPECT_EQ(d.lookup("doex")->abbrev_mode, cbakey::core::AbbrevMode::En);
    EXPECT_EQ(d.lookup("btv")->abbrev_mode,  cbakey::core::AbbrevMode::Both);
    EXPECT_EQ(d.lookup("old")->abbrev_mode,  cbakey::core::AbbrevMode::Vi);  // backward compat
    fs::remove(p);
}

TEST(M13Schema, UpsertAndRemove) {
    cbakey::core::UserDict d;
    d.upsert({"a", "alpha", cbakey::core::AbbrevMode::Vi});
    d.upsert({"b", "beta",  cbakey::core::AbbrevMode::En});
    EXPECT_EQ(d.size(), 2u);

    const bool replaced = d.upsert({"a", "ALPHA", cbakey::core::AbbrevMode::Both});
    EXPECT_TRUE(replaced);
    EXPECT_EQ(d.lookup("a")->expansion, "ALPHA");
    EXPECT_EQ(d.lookup("a")->abbrev_mode, cbakey::core::AbbrevMode::Both);

    EXPECT_TRUE(d.remove("b"));
    EXPECT_FALSE(d.remove("b"));  // already gone
    EXPECT_EQ(d.size(), 1u);
}

TEST(M13Schema, SaveAndReload) {
    const std::string p = tmpDictPath() + ".save";
    cbakey::core::UserDict d;
    d.upsert({"ko",   "không",         cbakey::core::AbbrevMode::Vi});
    d.upsert({"doex", "docker exec",   cbakey::core::AbbrevMode::En});
    d.upsert({"btv",  "Ban Tổ chức",   cbakey::core::AbbrevMode::Both});
    ASSERT_TRUE(d.saveToFile(p));

    const auto d2 = cbakey::core::UserDict::loadFromFile(p);
    ASSERT_EQ(d2.size(), 3u);
    EXPECT_EQ(d2.lookup("ko")->expansion,   "không");
    EXPECT_EQ(d2.lookup("doex")->abbrev_mode, cbakey::core::AbbrevMode::En);
    EXPECT_EQ(d2.lookup("btv")->abbrev_mode,  cbakey::core::AbbrevMode::Both);
    fs::remove(p);
    fs::remove(p + ".bak");
}

TEST(M13Schema, SaveCreatesBackup) {
    const std::string p = tmpDictPath() + ".bak_test";
    writeDictFile(p, "{\"trigger\":\"x\",\"expansion\":\"y\"}\n");

    cbakey::core::UserDict d;
    d.upsert({"a", "b"});
    ASSERT_TRUE(d.saveToFile(p));

    EXPECT_TRUE(fs::exists(p + ".bak"));
    fs::remove(p);
    fs::remove(p + ".bak");
}

// ── Engine — Vi mode ─────────────────────────────────────────────────────────

TEST(M13Engine, ViModeEntryExpandsInVi) {
    const std::string p = tmpDictPath();
    writeDictFile(p, "{\"trigger\":\"ko\",\"expansion\":\"không\",\"abbrev_mode\":\"vi\"}\n");
    auto cfg = cbakey::config::defaultConfig();
    cfg.userDictPath = p;
    cfg.enableUserDictionary = true;
    cbakey::core::Engine eng(cfg);

    const std::string result = runSequence(eng, "ko ");
    EXPECT_EQ(result, "không ");
    fs::remove(p);
}

TEST(M13Engine, EnModeEntryDoesNotExpandInVi) {
    const std::string p = tmpDictPath();
    // Use a trigger that Telex won't transform (no tone/transform keys).
    writeDictFile(p, "{\"trigger\":\"cmd\",\"expansion\":\"command\",\"abbrev_mode\":\"en\"}\n");
    auto cfg = cbakey::config::defaultConfig();
    cfg.userDictPath = p;
    cfg.enableUserDictionary = true;
    cbakey::core::Engine eng(cfg);

    // "cmd" in Vi mode: c-m-d have no Telex tone/transform → commit as-is.
    // En-only entry must NOT expand in Vietnamese mode.
    const std::string result = runSequence(eng, "cmd ");
    EXPECT_EQ(result, "cmd ");
    fs::remove(p);
}

TEST(M13Engine, BothModeEntryExpandsInVi) {
    const std::string p = tmpDictPath();
    writeDictFile(p, "{\"trigger\":\"btv\",\"expansion\":\"Ban Tổ chức\",\"abbrev_mode\":\"both\"}\n");
    auto cfg = cbakey::config::defaultConfig();
    cfg.userDictPath = p;
    cfg.enableUserDictionary = true;
    cbakey::core::Engine eng(cfg);

    const std::string result = runSequence(eng, "btv ");
    EXPECT_EQ(result, "Ban Tổ chức ");
    fs::remove(p);
}

// ── Engine — Password field ───────────────────────────────────────────────────

TEST(M13Engine, PasswordFieldSuppressesExpansion) {
    const std::string p = tmpDictPath();
    writeDictFile(p, "{\"trigger\":\"btv\",\"expansion\":\"Ban Tổ chức\"}\n");
    auto cfg = cbakey::config::defaultConfig();
    cfg.userDictPath = p;
    cfg.enableUserDictionary = true;
    cbakey::core::Engine eng(cfg);

    eng.setPasswordField(true);
    EXPECT_TRUE(eng.isPasswordField());

    const std::string result = runSequence(eng, "btv ");
    EXPECT_EQ(result, "btv ");  // not expanded

    eng.setPasswordField(false);
    eng.clearState();
    const std::string result2 = runSequence(eng, "btv ");
    EXPECT_EQ(result2, "Ban Tổ chức ");  // expanded again
    fs::remove(p);
}

// ── Engine — lookupEnglishAbbrev ─────────────────────────────────────────────

TEST(M13Engine, LookupEnglishAbbrev) {
    const std::string p = tmpDictPath();
    writeDictFile(p,
        "{\"trigger\":\"doex\",\"expansion\":\"docker exec\",\"abbrev_mode\":\"en\"}\n"
        "{\"trigger\":\"btv\",\"expansion\":\"Ban Tổ chức\",\"abbrev_mode\":\"both\"}\n"
        "{\"trigger\":\"ko\",\"expansion\":\"không\",\"abbrev_mode\":\"vi\"}\n");
    auto cfg = cbakey::config::defaultConfig();
    cfg.userDictPath = p;
    cfg.enableUserDictionary = true;
    cbakey::core::Engine eng(cfg);

    // En and Both entries are reachable via lookupEnglishAbbrev
    ASSERT_NE(eng.lookupEnglishAbbrev("doex"), nullptr);
    EXPECT_EQ(eng.lookupEnglishAbbrev("doex")->expansion, "docker exec");
    ASSERT_NE(eng.lookupEnglishAbbrev("btv"), nullptr);

    // Vi-only entry is NOT reachable via lookupEnglishAbbrev
    EXPECT_EQ(eng.lookupEnglishAbbrev("ko"), nullptr);

    // Password field blocks lookup
    eng.setPasswordField(true);
    EXPECT_EQ(eng.lookupEnglishAbbrev("doex"), nullptr);
    fs::remove(p);
}

// ── Backward compatibility ────────────────────────────────────────────────────

TEST(M13BackwardCompat, M8EntriesWithoutModeStillExpand) {
    // Entries without "abbrev_mode" (M8 format) default to Vi and expand normally.
    const std::string p = tmpDictPath();
    writeDictFile(p,
        "{\"trigger\":\"dk\",\"expansion\":\"được\"}\n"
        "{\"trigger\":\"mk\",\"expansion\":\"mình\"}\n");
    auto cfg = cbakey::config::defaultConfig();
    cfg.userDictPath = p;
    cfg.enableUserDictionary = true;
    cbakey::core::Engine eng(cfg);

    EXPECT_EQ(runSequence(eng, "dk "), "được ");
    eng.clearState();
    EXPECT_EQ(runSequence(eng, "mk "), "mình ");
    fs::remove(p);
}
