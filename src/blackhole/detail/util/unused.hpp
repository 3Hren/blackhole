#pragma once

#include "blackhole/detail/config/inline.hpp"

namespace blackhole {

namespace aux {

namespace util {

template<typename... Args>
BLACKHOLE_ALWAYS_INLINE
inline void ignore_unused_variable_warning(const Args&...) {}

} // namespace util

} // namespace aux

} // namespace blackhole
