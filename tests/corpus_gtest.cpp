#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

#include "corpus_driver.h"

namespace {

void trimInPlace(std::string* s) {
    while (!s->empty() && (s->back() == '\r' || s->back() == '\n' || s->back() == ' ' || s->back() == '\t')) {
        s->pop_back();
    }
    std::size_t i = 0;
    while (i < s->size() && (s->at(i) == ' ' || s->at(i) == '\t')) {
        ++i;
    }
    s->erase(0, i);
}

void runJsonlFile(const fs::path& filePath, farolkey::test::CorpusRunStats* total) {
    farolkey::test::CorpusRunStats fileStats;
    std::ifstream in(filePath);
    ASSERT_TRUE(in.good()) << "cannot open " << filePath;

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        trimInPlace(&line);
        if (line.empty()) {
            continue;
        }

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(line);
        } catch (const nlohmann::json::parse_error& e) {
            FAIL() << filePath.string() << ":" << lineNo << " JSON parse error: " << e.what();
        }

        const std::string id = j.value("id", std::string{"<no-id>"});
        SCOPED_TRACE(filePath.filename().string() + ":" + std::to_string(lineNo) + " id=" + id);

        const farolkey::test::CorpusOutcome outcome = farolkey::test::runCorpusCase(j, &fileStats);

        if (outcome.skipped) {
            std::cerr << "[SKIP] " << filePath.filename().string() << ":" << lineNo << " id=" << id;
            if (j.contains("skip_reason")) {
                std::cerr << " reason=" << j["skip_reason"].get<std::string>();
            }
            std::cerr << '\n';
            continue;
        }

        ASSERT_FALSE(outcome.error.has_value())
            << filePath.string() << ":" << lineNo << " " << *outcome.error;
    }

    total->executed += fileStats.executed;
    total->skipped += fileStats.skipped;
    std::cerr << "[corpus] " << filePath.filename().string() << " executed=" << fileStats.executed
              << " skipped=" << fileStats.skipped << '\n';
}

}  // namespace

TEST(CorpusJsonl, AllFilesUnderCorpus) {
    const fs::path root = "corpus";
    ASSERT_TRUE(fs::exists(root)) << "working directory must be project root (corpus/ missing)";
    ASSERT_TRUE(fs::is_directory(root));

    fs::path scanRoot = root;
    const fs::path finalRoot = root / "final";
    if (fs::exists(finalRoot) && fs::is_directory(finalRoot)) {
        scanRoot = finalRoot;
    }

    std::vector<fs::path> files;
    std::error_code ec;
    for (const fs::directory_entry& e :
         fs::recursive_directory_iterator(scanRoot, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) {
            break;
        }
        if (!e.is_regular_file()) {
            continue;
        }
        if (e.path().extension() == ".jsonl") {
            files.push_back(e.path());
        }
    }
    std::sort(files.begin(), files.end());

    ASSERT_FALSE(files.empty()) << "no .jsonl files under corpus/";

    farolkey::test::CorpusRunStats total;
    for (const fs::path& p : files) {
        runJsonlFile(p, &total);
    }
    std::cerr << "[corpus] root=" << scanRoot.string() << " TOTAL executed=" << total.executed
              << " skipped=" << total.skipped << '\n';
}
