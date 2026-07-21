#include <stddef.h>

#include <error.h>

#include <cpptypr/error.hpp>

#include <ostream>

namespace cpptypr {

Error::Error(ErrorCode code)
    : std::runtime_error([code] {
        char buf[128];
        ::engineErrorToString(static_cast<::EngineError>(code), buf, sizeof(buf));
        return std::string(buf);
    }()), m_code(code) {}

std::ostream& operator<<(std::ostream& os, ErrorCode code) {
    switch (code) {
        case ErrorCode::None:           return os << "none";
        case ErrorCode::InvalidMode:    return os << "invalid_mode";
        case ErrorCode::InvalidTimeout: return os << "invalid_timeout";
        case ErrorCode::AlreadyRunning: return os << "already_running";
        case ErrorCode::NotRunning:     return os << "not_running";
        case ErrorCode::Config:         return os << "config";
        case ErrorCode::Content:        return os << "content";
        case ErrorCode::State:          return os << "state";
        case ErrorCode::Provider:       return os << "provider";
        case ErrorCode::File:           return os << "file";
        case ErrorCode::Unknown:        return os << "unknown";
    }
    return os << "unknown";
}

std::ostream& operator<<(std::ostream& os, const Error& err) {
    return os << "Error(" << err.code() << "): " << err.what();
}

}
