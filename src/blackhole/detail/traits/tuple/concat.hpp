#pragma once

#include <tuple>

namespace blackhole {

namespace tuple {

template<class Sequence, class OtherSequence>
struct concat;

template<typename... Sequence, typename... OtherSequence>
struct concat<std::tuple<Sequence...>, std::tuple<OtherSequence...>> {
    typedef std::tuple<Sequence..., OtherSequence...> type;
};

} // namespace tuple

} // namespace blackhole
