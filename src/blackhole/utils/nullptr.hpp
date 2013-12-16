#pragma once

#include "blackhole/platform.hpp"

#if !defined(__clang__) && !defined(HAVE_GCC46)
    #define nullptr __null
#endif
