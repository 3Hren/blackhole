#pragma once

#include <type_traits>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

template<class T>
struct is_true_type {
    static const bool value = std::is_same<T, std::true_type>::value;
};

BLACKHOLE_END_NS
