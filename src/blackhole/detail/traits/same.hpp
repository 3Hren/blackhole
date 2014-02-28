#pragma once

#include <type_traits>

namespace blackhole {

namespace aux {

template<class...>
struct are_same;

template<class T>
struct are_same<T> : public std::true_type {};

template<class T, class Arg, class... Args>
struct are_same<T, Arg, Args...> {
    static const bool value = std::is_same<T, Arg>::value && are_same<T, Args...>::value;
};

} // namespace aux

} // namespace blackhole
