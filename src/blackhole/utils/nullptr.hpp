#pragma once

#include "blackhole/platform.hpp"

#if !defined(__clang__) && !defined(BLACKHOLE_HAVE_AT_LEAST_GCC46)
    #define nullptr __null
#endif
