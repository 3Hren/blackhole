#pragma once

#include <type_traits>

#include "blackhole/config.hpp"

#include "blackhole/detail/traits/or.hpp"

BLACKHOLE_BEG_NS

template<typename T>
struct is_string_literal_type : public or_<
    typename std::is_same<
        const char*,
        typename std::decay<T>::type
    >::type,
    typename std::is_same<
        char*,
        typename std::decay<T>::type
    >::type
> {};

BLACKHOLE_END_NS
