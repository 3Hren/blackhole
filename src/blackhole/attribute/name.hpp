#pragma once

#include <string>

#include "blackhole/platform/compiler.hpp"

namespace blackhole {

namespace attribute {

typedef std::string name_t;

} // namespace attribute

#if defined(BLACKHOLE_HAVE_CLANG) || defined(BLACKHOLE_HAVE_AT_LEAST_GCC46)
typedef attribute::name_t attribute_name_t __attribute__((deprecated("Use `attribute::name_t` instead.")));
#else
typedef attribute::name_t attribute_name_t __attribute__((deprecated));
#endif

} // namespace blackhole
