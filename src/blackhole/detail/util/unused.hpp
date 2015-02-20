#pragma once

#include "blackhole/config.hpp"

#include "blackhole/detail/config/inline.hpp"

BLACKHOLE_BEG_NS

namespace aux {

namespace util {

template<typename... Args>
BLACKHOLE_ALWAYS_INLINE
inline void ignore_unused_variable_warning(const Args&...) {}

} // namespace util

} // namespace aux

BLACKHOLE_END_NS
