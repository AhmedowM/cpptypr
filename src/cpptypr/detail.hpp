#pragma once

#include <cctype>
#include <string>
#include <string_view>

#include <cpptypr/error.hpp>

#define CHECK_MOVED() do { if (!m_impl) throw cpptypr::Error(cpptypr::ErrorCode::State); } while(0)

namespace cpptypr { namespace detail {

inline std::string toLower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (auto c : s) { out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c)))); }
    return out;
}

}}
