#pragma once

#include <type_traits>
namespace blackhole {

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

} // namespace blackhole
