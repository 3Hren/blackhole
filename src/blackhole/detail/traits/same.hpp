#pragma once

#include <type_traits>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace aux {

template<class...>
struct are_same;

template<class T>
struct are_same<T> : public std::true_type {};

template<class T, class Arg, class... Args>
struct are_same<T, Arg, Args...> :
    public std::conditional<
        std::is_same<T, Arg>::value && are_same<T, Args...>::value,
        std::true_type,
        std::false_type
    >::type
{};

} // namespace aux

BLACKHOLE_END_NS
