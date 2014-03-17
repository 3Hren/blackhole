#pragma once

#include <ostream>
#include <type_traits>

#include "blackhole/platform.hpp"

namespace blackhole {

namespace support {

template<typename T>
typename std::add_rvalue_reference<T>::type declval();

} // namespace support

namespace traits {

namespace supports {

namespace aux {

struct return_t {};

struct any_t {
    template<typename T> any_t(const T&);
};

} // namespace aux

aux::return_t operator<<(std::ostream&, const aux::any_t&);

template<typename T>
struct stream_push : public std::integral_constant<
        bool,
        !std::is_same<
            aux::return_t,
            decltype(support::declval<std::ostream&>() << support::declval<T>())
        >::value
    >
{};

} // namespace supports

} // traits

} // namespace blackhole
