#pragma once

#include <type_traits>
#include <tuple>

#include "blackhole/config.hpp"

#include "blackhole/detail/traits/tuple/concat.hpp"

BLACKHOLE_BEG_NS

namespace tuple {

template<template<typename> class F, class Sequence>
struct filter;

template<template<typename> class F>
struct filter<F, std::tuple<>> {
    typedef std::tuple<> type;
};

template<template<typename> class F, typename T, typename... Args>
struct filter<F, std::tuple<T, Args...>> {
    typedef typename std::conditional<
        F<T>::value,
        typename concat<std::tuple<T>, typename filter<F, std::tuple<Args...>>::type>::type,
        typename filter<F, std::tuple<Args...>>::type
    >::type type;
};

} // namespace tuple

BLACKHOLE_END_NS
