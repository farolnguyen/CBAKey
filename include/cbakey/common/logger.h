#pragma once

#include <string>

namespace cbakey::common {

enum class LogLevel {
    Error,
    Warning,
    Info
};

void log(LogLevel level, const std::string& component, const std::string& message);

}  // namespace cbakey::common
