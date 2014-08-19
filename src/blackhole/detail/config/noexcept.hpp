#pragma once

#include "platform/noexcept.hpp"

#if defined(BLACKHOLE_HAS_FEATURE_NOEXCEPT)
    #define BLACKHOLE_NOEXCEPT noexcept
#else
    #define BLACKHOLE_NOEXCEPT throw()
#endif
