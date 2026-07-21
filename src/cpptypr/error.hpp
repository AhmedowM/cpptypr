#pragma once

#include <iosfwd>
#include <stdexcept>
#include <string>

namespace cpptypr {

/** @brief Error codes returned or thrown by cpptypr operations. */
enum class ErrorCode {
    None,            /**< No error. */
    InvalidMode,     /**< An invalid engine mode was provided. */
    InvalidTimeout,  /**< An invalid timeout value was provided. */
    AlreadyRunning,  /**< Operation failed because the engine is already running. */
    NotRunning,      /**< Operation failed because the engine is not running. */
    Config,          /**< Engine configuration error. */
    Content,         /**< Content provider error. */
    State,           /**< Invalid state transition. */
    Provider,        /**< Provider initialization or access error. */
    File,            /**< File I/O error. */
    Unknown          /**< Unspecified error. */
};

/** @brief Exception thrown when a cpptypr operation fails. */
class Error : public std::runtime_error {
public:
    /** @brief Construct an error with the given code.
     *  @param code The error code describing the failure. */
    explicit Error(ErrorCode code);

    /** @brief Returns the error code.
     *  @return The ErrorCode value passed at construction. */
    [[nodiscard]] ErrorCode code() const noexcept { return m_code; }

private:
    ErrorCode m_code;
};

/** @brief Write an ErrorCode to an output stream.
 *  @param os   The output stream.
 *  @param code The error code.
 *  @return The output stream. */
std::ostream& operator<<(std::ostream& os, ErrorCode code);

/** @brief Write an Error to an output stream.
 *  @param os  The output stream.
 *  @param err The error.
 *  @return The output stream. */
std::ostream& operator<<(std::ostream& os, const Error& err);

}
