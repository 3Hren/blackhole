#pragma once

#include "blackhole/detail/config/inline.hpp"

namespace blackhole {

namespace aux {

inline
bool
BLACKHOLE_ALWAYS_INLINE
__attribute__((format(__printf__, 1, 2)))
syntax_check(const char*, ...) {
    return true;
}

} // namespace aux

} // namespace blackhole
