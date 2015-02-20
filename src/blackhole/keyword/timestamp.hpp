#pragma once

#include <sys/time.h>

#include "blackhole/keyword.hpp"

BLACKHOLE_BEG_NS

DECLARE_EVENT_KEYWORD(timestamp, timeval)

namespace keyword {

namespace init {

static inline timeval timestamp() {
    timeval tv;
    const int res = gettimeofday(&tv, nullptr);
    if (res == 0) {
        return tv;
    } else {
        return timeval { 0, 0 };
    }
}

} // namespace init

} // namespace keyword

BLACKHOLE_END_NS
