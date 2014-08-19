#pragma once

#include "platform/atomic.hpp"

#if defined(BLACKHOLE_HAS_FEATURE_ATOMIC)
    #include <atomic>
#else
    #include <cstdatomic>
#endif
