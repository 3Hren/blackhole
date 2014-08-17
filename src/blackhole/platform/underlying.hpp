#pragma once

#include "compiler.hpp"

#if (defined(BLACKHOLE_HAVE_CLANG) && defined(TARGET_OS_MAC))|| defined(BLACKHOLE_HAVE_GCC47)
    #define BLACKHOLE_HAS_FEATURE_UNDERLYING_TYPE
#endif
