#pragma once

#include "platform/constexpr.hpp"

#if defined(BLACKHOLE_HAS_FEATURE_CONSTEXPR)
    #define BLACKHOLE_CONSTEXPR constexpr
#else
    #define BLACKHOLE_CONSTEXPR
#endif
