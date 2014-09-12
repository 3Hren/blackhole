#pragma once

namespace blackhole {

namespace aux {

namespace util {

template<typename T>
struct lazy_false {
    static const bool value = sizeof(T) == -1;
};

} // namespace util

} // namespace aux

} // namespace blackhole
