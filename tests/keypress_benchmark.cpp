#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"

namespace {

struct BenchScenario {
    const char* name;
    std::vector<std::string> sequences;  // each string is one word sequence
};

double runScenario(const BenchScenario& s, const cbakey::config::RuntimeConfig& cfg,
                   int iterations) {
    cbakey::core::Engine engine(cfg);
    long long keyCount = 0;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (const std::string& seq : s.sequences) {
            engine.clearState();
            for (const char key : seq) {
                engine.processKey(cbakey::core::KeyEvent{.key = key});
                ++keyCount;
            }
            engine.processKey(cbakey::core::KeyEvent{.key = ' '});
            ++keyCount;
        }
    }
    const auto end = std::chrono::steady_clock::now();
    const long long elapsedNs =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    const double avgUs = static_cast<double>(elapsedNs) / static_cast<double>(keyCount) / 1000.0;
    std::cout << "  " << s.name << ": " << keyCount << " keys, avg " << avgUs << " µs/key\n";
    return avgUs;
}

}  // namespace

int main() {
    constexpr int kIter = 100000;

    // Budget: <10 µs/key is acceptable; <2 µs/key is target for hot path.

    const BenchScenario telex{
        "Telex (tones + transforms)",
        {"tieengs", "vieetj", "ddangx", "thuwr", "xin", "chaof",
         "nguwowif", "hoocj", "sin", "chuaanr"}};

    const BenchScenario vni{
        "VNI (digits + transforms)",
        {"tie61ng", "vie65t", "d9a5ng", "thu7", "xin", "cha2o",
         "ngu7o71i", "ho9c", "xua6n3"}};

    const BenchScenario longWord{
        "Long multisyllable auto-commit",
        {"xinchao", "thuo61cba", "nguyeen"}};

    const BenchScenario dictHit{
        "Dict expansion (no-op when dict empty)",
        {"ko", "btv", "dk"}};

    cbakey::config::RuntimeConfig telexCfg = cbakey::config::defaultConfig();
    cbakey::config::RuntimeConfig vniCfg = cbakey::config::defaultConfig();
    vniCfg.method = cbakey::core::InputMethod::Vni;

    std::cout << "=== CBAKey keypress benchmark (Release) ===\n";
    std::cout << "Budget: <2 µs/key target, <10 µs/key acceptable\n\n";

    const double t1 = runScenario(telex, telexCfg, kIter);
    const double t2 = runScenario(vni, vniCfg, kIter);
    const double t3 = runScenario(longWord, telexCfg, kIter);
    const double t4 = runScenario(dictHit, telexCfg, kIter);

    const double worst = std::max({t1, t2, t3, t4});
    std::cout << "\nWorst-case: " << worst << " µs/key\n";
    if (worst < 2.0) {
        std::cout << "✓ PASS — within 2 µs/key target\n";
    } else if (worst < 10.0) {
        std::cout << "~ OK — within 10 µs/key acceptable limit (consider optimization)\n";
    } else {
        std::cout << "✗ FAIL — exceeds 10 µs/key budget\n";
        return 1;
    }
    return 0;
}
