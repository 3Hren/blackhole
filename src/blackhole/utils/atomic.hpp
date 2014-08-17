#pragma once

#include "blackhole/platform.hpp"

#if (defined(BLACKHOLE_HAS_CLANG) && defined(TARGET_OS_MAC)) || defined(BLACKHOLE_HAS_AT_LEAST_GCC46)
    #include <atomic>
#else
    #include <cstdatomic>
#endif
