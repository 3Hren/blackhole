#pragma once

#include "blackhole/platform.hpp"

#if !defined(BLACKHOLE_HAS_CLANG) && !defined(BLACKHOLE_HAS_AT_LEAST_GCC46)
    #define nullptr __null
#endif
