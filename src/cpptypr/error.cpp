#include <error.h>

#include <cpptypr/error.hpp>

namespace cpptypr {

Error::Error(ErrorCode code)
    : std::runtime_error([code] {
        char buf[128];
        ::engineErrorToString(static_cast<::EngineError>(code), buf, sizeof(buf));
        return std::string(buf);
    }()), m_code(code) {}

}
