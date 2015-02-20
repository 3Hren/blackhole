#pragma once

#include <type_traits>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace sink {

namespace thread {

enum class safety_t { unsafe = 0, safe };

} // namespace thread

template<class T>
struct thread_safety :
    public std::integral_constant<
        thread::safety_t,
        thread::safety_t::unsafe
    >::type
{};

} // namespace sink

BLACKHOLE_END_NS
