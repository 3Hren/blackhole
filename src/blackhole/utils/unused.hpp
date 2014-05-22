#pragma once

namespace blackhole {

namespace utils {

template<typename... Args>
inline void ignore_unused_variable_warning(const Args&...) {}

} // namespace utils

} // namespace blackhole
