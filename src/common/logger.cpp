#include "cbakey/common/logger.h"

#include <iostream>

namespace cbakey::common {

namespace {

const char* toString(LogLevel level) {
    switch (level) {
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Info:
            return "INFO";
    }
    return "INFO";
}

}  // namespace

void log(LogLevel level, const std::string& component, const std::string& message) {
    // Strict privacy: caller must never pass raw user typing content here.
    std::cerr << "[" << toString(level) << "]"
              << "[" << component << "] " << message << '\n';
}

}  // namespace cbakey::common
