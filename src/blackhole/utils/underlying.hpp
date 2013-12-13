#pragma once

#include <type_traits>

namespace blackhole {

namespace aux {

template<typename T, class = void>
struct underlying_type {
    typedef T type;
};

template<typename T>
struct underlying_type<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename std::underlying_type<T>::type type;
};

} // namespace aux

} // namespace blackhole
