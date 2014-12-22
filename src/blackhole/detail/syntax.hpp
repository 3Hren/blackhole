#pragma once

#include "blackhole/config.hpp"
#include "blackhole/detail/config/inline.hpp"

namespace blackhole {

namespace aux {

inline
bool
BLACKHOLE_ALWAYS_INLINE
#if BLACKHOLE_CHECK_FORMAT_SYNTAX == 1
__attribute__((format(__printf__, 1, 2)))
#endif
syntax_check(const char*, ...) {
    return true;
}

} // namespace aux

} // namespace blackhole
