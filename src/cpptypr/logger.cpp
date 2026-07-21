#include <logger.h>

#include <cpptypr/logger.hpp>
#include <cpptypr/detail.hpp>

void LoggerDeleter::operator()(::Logger* p) const noexcept { ::loggerDestroy(p); }

#include <ostream>

namespace cpptypr {

std::string_view toString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:   return "debug";
        case LogLevel::Info:    return "info";
        case LogLevel::Warning: return "warning";
        case LogLevel::Error:   return "error";
        case LogLevel::None:    return "none";
    }
    return "debug";
}

LogLevel logLevelFromString(std::string_view s) {
    auto lower = cpptypr::detail::toLower(s);
    if (lower == "debug")   return LogLevel::Debug;
    if (lower == "info")    return LogLevel::Info;
    if (lower == "warning") return LogLevel::Warning;
    if (lower == "error")   return LogLevel::Error;
    if (lower == "none")    return LogLevel::None;
    throw Error(ErrorCode::Config);
}

std::ostream& operator<<(std::ostream& os, LogLevel level) {
    return os << toString(level);
}

Logger::Logger(LogLevel level, bool enableStdout)
    : m_impl(::loggerCreate(static_cast<::LogLevel>(level), enableStdout)) {}

Logger::Logger(std::string_view level, bool enableStdout)
    : Logger(logLevelFromString(level), enableStdout) {}

Logger::~Logger() = default;

Logger::Logger(Logger&&) noexcept = default;

Logger& Logger::operator=(Logger&&) noexcept = default;

void Logger::setLevel(LogLevel level) { CHECK_MOVED(); ::loggerSetLevel(m_impl.get(), static_cast<::LogLevel>(level)); }
void Logger::setLevel(std::string_view level) { CHECK_MOVED(); setLevel(logLevelFromString(level)); }

LogLevel Logger::level() const { CHECK_MOVED(); return static_cast<LogLevel>(::loggerGetLevel(m_impl.get())); }

void Logger::logToStdout(bool enable) { CHECK_MOVED(); ::loggerLogToStdout(m_impl.get(), enable); }

bool Logger::addFile(std::string_view filepath) {
    CHECK_MOVED();
    return ::loggerAddFile(m_impl.get(), std::string(filepath).c_str());
}

void Logger::log(LogLevel level, std::string_view message) {
    CHECK_MOVED();
    ::loggerLog(m_impl.get(), static_cast<::LogLevel>(level), std::string(message).c_str());
}

void Logger::log(std::string_view level, std::string_view message) {
    CHECK_MOVED();
    log(logLevelFromString(level), message);
}

}
