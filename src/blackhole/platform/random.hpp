#pragma once

#include <blackhole/platform.hpp>

#if defined(__clang__) || defined(HAVE_GCC46)
#define HAS_CXX11_RANDOM
#endif
