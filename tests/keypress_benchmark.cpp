#include <chrono>
#include <iostream>
#include <string>

#include "cbakey/config/config.h"
#include "cbakey/core/engine.h"

int main() {
    cbakey::core::Engine engine(cbakey::config::defaultConfig());
    constexpr int kIterations = 100000;
    const std::string sample = "tieengs vieetj ddangx thuwr";

    const auto start = std::chrono::steady_clock::now();
    int keyCount = 0;
    for (int i = 0; i < kIterations; ++i) {
        engine.clearState();
        for (const char key : sample) {
            engine.processKey(cbakey::core::KeyEvent{.key = key});
            ++keyCount;
        }
        engine.processKey(cbakey::core::KeyEvent{.key = ' '});
        ++keyCount;
    }
    const auto end = std::chrono::steady_clock::now();
    const auto elapsedNs =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    const double avgNs = static_cast<double>(elapsedNs) / static_cast<double>(keyCount);
    const double avgUs = avgNs / 1000.0;

    std::cout << "Processed keys: " << keyCount << '\n';
    std::cout << "Total time (ns): " << elapsedNs << '\n';
    std::cout << "Average latency per key (us): " << avgUs << '\n';
    return 0;
}
