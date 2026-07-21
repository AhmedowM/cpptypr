#include <core/version.h>

#include <cpptypr/version.hpp>

namespace ctypr {

const uint32_t Version::Major = CTYPR_VERSION_MAJOR;
const uint32_t Version::Minor = CTYPR_VERSION_MINOR;
const uint32_t Version::Patch = CTYPR_VERSION_PATCH;
const std::string_view Version::operator()() { return CTYPR_VERSION_STRING; }

}
