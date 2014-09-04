#pragma once

#include <array>
#include <type_traits>

#include "blackhole/detail/traits/same.hpp"

namespace blackhole {

namespace aux {

template<typename T, typename... Args>
typename std::enable_if<
    are_same<T, Args...>::value,
    std::array<T, sizeof...(Args) + 1>
>::type
make_array(T t, Args&&... args) {
    return std::array<T, sizeof...(Args) + 1> {{ t, std::forward<Args>(args)... }};
}

} // namespace utils

} // namespace blackhole
