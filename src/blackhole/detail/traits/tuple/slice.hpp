#pragma once

#include <type_traits>
#include <tuple>

#include <boost/integer_traits.hpp>

namespace blackhole {

namespace tuple {

template<int START, int STOP = -1, int STEP = 1>
struct slice {
    template<class IndexedSequence>
    struct type;

    template<int N, typename T>
    struct type<std::tuple<std::integral_constant<int, N>, T>> {
        static const int delta = N - START;
        static const int stop = STOP == -1 ? boost::integer_traits<int>::const_max : STOP;
        static const bool value = delta >= 0 && N < stop && delta % STEP == 0;
    };
};

} // namespace tuple

} // namespace blackhole
