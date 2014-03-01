#pragma once

#include <type_traits>
#include <tuple>

#include "blackhole/detail/traits/tuple/concat.hpp"

namespace blackhole {

namespace tuple {

template<template<typename> class F, class Sequence>
struct filter_impl;

template<template<typename> class F>
struct filter_impl<F, std::tuple<>> {
    typedef std::tuple<> type;
};

template<template<typename> class F, typename T, typename... Args>
struct filter_impl<F, std::tuple<T, Args...>> {
    typedef typename std::conditional<
        F<T>::value,
        typename concat<std::tuple<T>, typename filter_impl<F, std::tuple<Args...>>::type>::type,
        typename filter_impl<F, std::tuple<Args...>>::type
    >::type type;
};

template<template<typename> class F, class Sequence>
struct filter;

template<template<typename> class F, typename... Args>
struct filter<F, std::tuple<Args...>> {
    typedef typename filter_impl<F, std::tuple<Args...>>::type type;
};

} // namespace tuple

} // namespace blackhole
