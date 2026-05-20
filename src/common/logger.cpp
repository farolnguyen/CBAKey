#include "farolkey/common/logger.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>

namespace farolkey::common {
namespace {

constexpr std::uintmax_t kMaxLogBytes   = 5 * 1024 * 1024;  // 5 MB
constexpr const char*    kLogFileName   = "farolkey.log";
constexpr const char*    kLogFileBackup = "farolkey.log.1";

std::mutex       g_mutex;
std::ofstream    g_file;
bool             g_initialized = false;

std::filesystem::path logDir() {
    const char* xdg = std::getenv("XDG_CACHE_HOME");
    std::filesystem::path base =
        (xdg && xdg[0]) ? std::filesystem::path(xdg)
                         : std::filesystem::path(std::getenv("HOME") ? std::getenv("HOME") : ".")
                               / ".cache";
    return base / "farolkey";
}

void rotateIfNeeded(const std::filesystem::path& path) {
    std::error_code ec;
    if (std::filesystem::file_size(path, ec) > kMaxLogBytes && !ec) {
        std::filesystem::path bak = path.parent_path() / kLogFileBackup;
        std::filesystem::rename(path, bak, ec);
    }
}

void ensureOpen() {
    if (g_initialized) return;
    g_initialized = true;
    try {
        std::filesystem::path dir = logDir();
        std::filesystem::create_directories(dir);
        std::filesystem::path logPath = dir / kLogFileName;
        rotateIfNeeded(logPath);
        g_file.open(logPath, std::ios::app);
    } catch (...) {
        // If we can't open the log file, fall back to stderr silently.
    }
}

const char* levelStr(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
    }
    return "INFO ";
}

std::string timestamp() {
    using namespace std::chrono;
    auto now   = system_clock::now();
    auto tt    = system_clock::to_time_t(now);
    auto ms    = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm{};
    localtime_r(&tt, &tm);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<int>(ms.count()));
    return buf;
}

}  // namespace

void log(LogLevel level, const std::string& component, const std::string& message) {
    // PRIVACY: this function must never receive user-typed text.
    std::lock_guard<std::mutex> lock(g_mutex);
    ensureOpen();

    std::string line = timestamp() + " [" + levelStr(level) + "] [" + component + "] " + message + "\n";

    if (g_file.is_open()) {
        g_file << line;
        g_file.flush();
    } else {
        std::cerr << line;
    }
}

}  // namespace farolkey::common
