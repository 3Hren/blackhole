#pragma once

#include "compiler.hpp"

#if defined(__clang__) || defined(BLACKHOLE_HAVE_AT_LEAST_GCC46)
    #define BLACKHOLE_HAS_CXX11_RANDOM
#endif
