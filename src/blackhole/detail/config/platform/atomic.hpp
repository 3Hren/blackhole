#pragma once

#include "compiler.hpp"

#if (defined(BLACKHOLE_HAS_CLANG) && defined(TARGET_OS_MAC)) || defined(BLACKHOLE_HAS_AT_LEAST_GCC46)
    #define BLACKHOLE_HAS_FEATURE_ATOMIC
#endif
