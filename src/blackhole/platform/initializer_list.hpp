#pragma once

#include "compiler.hpp"

#if defined(__clang__) || defined(BLACKHOLE_HAVE_AT_LEAST_GCC46)
    #define BLACKHOLE_INITIALIZER_LIST_HAS_TYPEDEFS
#endif
