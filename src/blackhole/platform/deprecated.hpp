#pragma once

#include "blackhole/platform/compiler.hpp"

#if defined(BLACKHOLE_HAVE_CLANG) || defined(BLACKHOLE_HAVE_AT_LEAST_GCC46)
    #define BLACKHOLE_DEPRECATED(__msg__) __attribute__((deprecated(__msg__)))
#else
    #define BLACKHOLE_DEPRECATED(__msg__) __attribute__((deprecated))
#endif
