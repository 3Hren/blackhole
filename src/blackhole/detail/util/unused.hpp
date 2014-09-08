#pragma once

namespace blackhole {

namespace aux {

namespace util {

template<typename... Args>
__attribute__((always_inline))
inline void ignore_unused_variable_warning(const Args&...) {}

} // namespace util

} // namespace aux

} // namespace blackhole
