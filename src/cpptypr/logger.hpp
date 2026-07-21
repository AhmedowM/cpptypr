#pragma once

#include <iosfwd>
#include <memory>
#include <string_view>

#include <cpptypr/error.hpp>

struct Logger;
struct LoggerDeleter { void operator()(::Logger* p) const noexcept; };

namespace cpptypr {

class ContentProvider;
class Repository;
class Engine;

/** @brief Log severity levels. Higher values are more severe. */
enum class LogLevel {
    Debug,   /**< Detailed debug information. */
    Info,    /**< Informational messages. */
    Warning, /**< Potentially harmful situations. */
    Error,   /**< Error events that might still allow the application to continue. */
    None     /**< No logging. */
};

/** @brief Convert a LogLevel to its lowercase string representation.
 *  @param level The log level.
 *  @return "debug", "info", "warning", "error", or "none". */
[[nodiscard]] std::string_view toString(LogLevel level) noexcept;

/** @brief Parse a case-insensitive string to a LogLevel.
 *  @param s One of "debug", "info", "warning", "error", or "none".
 *  @return The matching LogLevel.
 *  @throws Error if the string does not match any valid level. */
[[nodiscard]] LogLevel logLevelFromString(std::string_view s);

/** @brief Write a LogLevel to an output stream.
 *  @param os    The output stream.
 *  @param level The log level.
 *  @return The output stream. */
std::ostream& operator<<(std::ostream& os, LogLevel level);

/** @brief RAII wrapper over the C logger.
 *
 * Manages a ::Logger instance with automatic cleanup on destruction.
 * Owns the underlying C logger — when the Logger object is destroyed,
 * the C logger is freed. */
class Logger {
public:
    /** @brief Construct a logger with the given severity level.
     *  @param level         Minimum log level (messages below this are suppressed).
     *  @param enableStdout  Whether to write log messages to stdout. */
    explicit Logger(LogLevel level = LogLevel::Debug, bool enableStdout = true);

    /** @brief Construct a logger with a string-based log level.
     *  @param level        Case-insensitive level ("debug", "info", "warning", "error", "none").
     *  @param enableStdout Whether to write log messages to stdout.
     *  @throws Error if the level string is invalid. */
    explicit Logger(std::string_view level, bool enableStdout = true);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) noexcept;
    Logger& operator=(Logger&&) noexcept;

    /** @brief Set the minimum log level.
     *  @param level Messages below this severity are suppressed. */
    void setLevel(LogLevel level);

    /** @brief Set the minimum log level using a case-insensitive string.
     *  @param level "debug", "info", "warning", "error", or "none".
     *  @throws Error if the string is invalid. */
    void setLevel(std::string_view level);

    /** @brief Get the current minimum log level.
     *  @return The active LogLevel. */
    [[nodiscard]] LogLevel level() const;

    /** @brief Enable or disable logging to stdout.
     *  @param enable true to write to stdout, false to suppress. */
    void logToStdout(bool enable);

    /** @brief Add a file output sink.
     *  @param filepath Path to the log file.
     *  @return true if the file was opened successfully, false otherwise. */
    [[nodiscard]] bool addFile(std::string_view filepath);

    /** @brief Log a message at the given severity.
     *  @param level   Severity of the message.
     *  @param message The message text. */
    void log(LogLevel level, std::string_view message);

    /** @brief Log a message at the given severity (string-based level).
     *  @param level   Case-insensitive severity ("debug", "info", "warning", "error", "none").
     *  @param message The message text.
     *  @throws Error if the level string is invalid. */
    void log(std::string_view level, std::string_view message);

private:
    friend class Engine;
    std::unique_ptr<::Logger, LoggerDeleter> m_impl;
};

}
