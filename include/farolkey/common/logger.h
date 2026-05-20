#pragma once

#include <string>

namespace farolkey::common {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

/// Write a log entry.
/// PRIVACY: callers MUST NOT pass any user-typed text or dictionary content.
/// Safe to log: char counts, state names, mode names, operation results, error messages.
void log(LogLevel level, const std::string& component, const std::string& message);

// Convenience helpers
inline void logDebug(const std::string& comp, const std::string& msg) {
    log(LogLevel::Debug, comp, msg);
}
inline void logInfo(const std::string& comp, const std::string& msg) {
    log(LogLevel::Info, comp, msg);
}
inline void logWarn(const std::string& comp, const std::string& msg) {
    log(LogLevel::Warning, comp, msg);
}
inline void logError(const std::string& comp, const std::string& msg) {
    log(LogLevel::Error, comp, msg);
}

}  // namespace farolkey::common
