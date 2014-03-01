#pragma once

#include <tuple>

namespace blackhole {

namespace tuple {

template<template<typename> class F, class Sequence>
struct map;

template<template<typename> class F, typename... Args>
struct map<F, std::tuple<Args...>>{
    typedef std::tuple<typename F<Args>::type...> type;
};

} // namespace tuple

} // namespace blackhole
