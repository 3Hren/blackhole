#pragma once

#include "compiler.hpp"

#if defined(BLACKHOLE_HAS_CLANG) || defined(BLACKHOLE_HAS_AT_LEAST_GCC46)
    #define BLACKHOLE_HAS_FEATURE_NOEXCEPT
#endif
