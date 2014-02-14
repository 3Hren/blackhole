#pragma once

#include "blackhole/platform.hpp"

#if defined(__clang__) || defined(HAVE_GCC46)
#define BLACKHOLE_NOEXCEPT noexcept
#else
#define BLACKHOLE_NOEXCEPT throw()
#endif
