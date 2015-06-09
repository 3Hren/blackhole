#pragma once

#include "blackhole/config.hpp"

#include "blackhole/detail/config/inline.hpp"

BLACKHOLE_BEG_NS

namespace aux {

#if BLACKHOLE_CHECK_FORMAT_SYNTAX == 1
    inline
    bool
    BLACKHOLE_ALWAYS_INLINE
    __attribute__((format(__printf__, 1, 2)))
    syntax_check(const char*, ...) {
        return true;
    }
#else
    template<typename... Args>
    inline
    bool
    BLACKHOLE_ALWAYS_INLINE
    syntax_check(const char*, Args&&...) {
        return true;
    }
#endif

} // namespace aux

BLACKHOLE_END_NS
