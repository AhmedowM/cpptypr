#include <logger.h>

#include <cpptypr/logger.hpp>
#include <cpptypr/detail.hpp>

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

namespace detail {

LogLevel parseLogLevel(std::string_view s) {
    auto lower = cpptypr::detail::toLower(s);
    if (lower == "debug")   return LogLevel::Debug;
    if (lower == "info")    return LogLevel::Info;
    if (lower == "warning") return LogLevel::Warning;
    if (lower == "error")   return LogLevel::Error;
    if (lower == "none")    return LogLevel::None;
    throw Error(ErrorCode::Config);
}

}

Logger::Logger(LogLevel level, bool enableStdout)
    : m_impl(::loggerCreate(static_cast<::LogLevel>(level), enableStdout)) {}

Logger::Logger(std::string_view level, bool enableStdout)
    : Logger(detail::parseLogLevel(level), enableStdout) {}

Logger::~Logger() { if (m_impl) { ::loggerDestroy(m_impl); } }

Logger::Logger(Logger&& other) noexcept : m_impl(other.m_impl) { other.m_impl = nullptr; }

Logger& Logger::operator=(Logger&& other) noexcept {
    if (this != &other) {
        if (m_impl) { ::loggerDestroy(m_impl); }
        m_impl = other.m_impl;
        other.m_impl = nullptr;
    }
    return *this;
}

void Logger::setLevel(LogLevel level) { CHECK_MOVED(); ::loggerSetLevel(m_impl, static_cast<::LogLevel>(level)); }
void Logger::setLevel(std::string_view level) { CHECK_MOVED(); setLevel(detail::parseLogLevel(level)); }

LogLevel Logger::level() const { CHECK_MOVED(); return static_cast<LogLevel>(::loggerGetLevel(m_impl)); }

void Logger::logToStdout(bool enable) { CHECK_MOVED(); ::loggerLogToStdout(m_impl, enable); }

bool Logger::addFile(std::string_view filepath) {
    CHECK_MOVED();
    return ::loggerAddFile(m_impl, std::string(filepath).c_str());
}

void Logger::log(LogLevel level, std::string_view message) {
    CHECK_MOVED();
    ::loggerLog(m_impl, static_cast<::LogLevel>(level), std::string(message).c_str());
}

void Logger::log(std::string_view level, std::string_view message) {
    CHECK_MOVED();
    log(detail::parseLogLevel(level), message);
}

}
