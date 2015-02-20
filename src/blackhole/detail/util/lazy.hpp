#pragma once

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace aux {

namespace util {

template<typename T>
struct lazy_false {
    static const bool value = sizeof(T) == -1;
};

} // namespace util

} // namespace aux

BLACKHOLE_END_NS
