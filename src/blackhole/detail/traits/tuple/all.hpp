#pragma once

#include <tuple>

#include "blackhole/detail/traits/is_true.hpp"

namespace blackhole {

namespace tuple {

template<class Sequence, template<class> class F = blackhole::is_true_type>
struct all;

template<template<class> class F>
struct all<std::tuple<>, F> : public std::true_type {};

template<template<class> class F, typename T, typename... Args>
struct all<std::tuple<T, Args...>, F> {
    static const bool value = !F<T>::value ? false : all<std::tuple<Args...>, F>::value;
};

} // namespace tuple

} // namespace blackhole
