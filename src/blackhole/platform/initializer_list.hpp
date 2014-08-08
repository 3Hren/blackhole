#pragma once

#include "compiler.hpp"

#if defined(__clang__) || defined(BH_HAVE_AT_LEAST_GCC_46)
#   define BH_INITIALIZER_LIST_HAS_TYPEDEFS
#endif
