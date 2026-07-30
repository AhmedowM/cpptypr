#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <string_view>

#ifdef NDEBUG
  #define CHECK_MOVED() do {} while(0)
#else
  #define CHECK_MOVED() do { assert(m_impl != nullptr); } while(0)
#endif

namespace cpptypr { namespace detail {

inline bool icaseEqual(std::string_view a, std::string_view b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(),
               [](unsigned char ca, unsigned char cb) { return std::tolower(ca) == std::tolower(cb); });
}

template<size_t N>
inline const char* nullTerminal(char (&buf)[N], std::string_view s) {
    size_t n = (s.size() < N - 1) ? s.size() : (N - 1);
    for (size_t i = 0; i < n; ++i) buf[i] = s[i];
    buf[n] = '\0';
    return buf;
}

}}
