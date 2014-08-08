#pragma once

#include "blackhole/platform.hpp"

#if defined(__clang__) || defined(BLACKHOLE_HAVE_AT_LEAST_GCC46)
#define BLACKHOLE_NOEXCEPT noexcept
#else
#define BLACKHOLE_NOEXCEPT throw()
#endif
