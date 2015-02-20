#pragma once

#include <tuple>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace tuple {

template<class Sequence, class OtherSequence>
struct concat;

template<typename... Sequence, typename... OtherSequence>
struct concat<std::tuple<Sequence...>, std::tuple<OtherSequence...>> {
    typedef std::tuple<Sequence..., OtherSequence...> type;
};

} // namespace tuple

BLACKHOLE_END_NS
