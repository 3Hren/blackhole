#pragma once

#include <type_traits>
#include <tuple>

#include "blackhole/config.hpp"

#include "blackhole/detail/traits/tuple/concat.hpp"

BLACKHOLE_BEG_NS

namespace tuple {

template<class IndexedSequence>
struct remove_index;

template<>
struct remove_index<std::tuple<>> {
    typedef std::tuple<> type;
};

template<int N, typename T, typename... Args>
struct remove_index<std::tuple<std::tuple<std::integral_constant<int, N>, T>, Args...>> {
    typedef typename concat<
        std::tuple<T>,
        typename remove_index<std::tuple<Args...>>::type
    >::type type;
};

} // namespace tuple

BLACKHOLE_END_NS
