#pragma once

#ifdef BLACKHOLE_HAS_ATTRIBUTE_LWP
#include <sys/syscall.h>
#endif

#include <string>

#include "blackhole/detail/util/thread.hpp"
#include "blackhole/keyword.hpp"

BLACKHOLE_BEG_NS

#ifdef BLACKHOLE_HAS_ATTRIBUTE_TID
DECLARE_KEYWORD(tid, std::string)
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_LWP
DECLARE_KEYWORD(lwp, uint64_t)
#endif

namespace keyword {

namespace init {

#ifdef BLACKHOLE_HAS_ATTRIBUTE_TID
static inline std::string tid() {
    return this_thread::get_id<std::string>();
}
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_LWP

static inline uint64_t lwp() {
#ifdef __linux__
    return ::syscall(SYS_gettid);
#elif __APPLE__
    return ::syscall(SYS_thread_selfid);
#else
#   error LWP attribute for your platform is not supported
#endif
}

#endif

} // namespace init

} // namespace keyword

BLACKHOLE_END_NS
