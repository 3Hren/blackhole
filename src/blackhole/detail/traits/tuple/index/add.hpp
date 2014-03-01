#pragma once

#include <type_traits>
#include <tuple>

#include "blackhole/detail/traits/tuple/concat.hpp"

namespace blackhole {

namespace tuple {

template<int N, int MAX, class Sequence>
struct add_index_impl;

template<int MAX>
struct add_index_impl<MAX, MAX, std::tuple<>> {
    typedef std::tuple<> type;
};

template<int N, int MAX, typename T, typename... Args>
struct add_index_impl<N, MAX, std::tuple<T, Args...>> {
    typedef typename concat<
        std::tuple<std::tuple<std::integral_constant<int, N>, T>>,
        typename add_index_impl<N + 1, MAX, std::tuple<Args...>>::type
    >::type type;
};

template<class Sequence>
struct add_index;

template<typename... Args>
struct add_index<std::tuple<Args...>> {
    typedef typename add_index_impl<0, sizeof...(Args), std::tuple<Args...>>::type type;
};

} // namespace tuple

} // namespace blackhole
