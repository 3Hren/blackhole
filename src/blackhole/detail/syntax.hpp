#pragma once

namespace blackhole {

namespace aux {

inline
bool
__attribute__((always_inline))
__attribute__((format(__printf__, 1, 2)))
syntax_check(const char*, ...) {
    return true;
}

} // namespace aux

} // namespace blackhole
