#pragma once

#include <ostream>
#include <type_traits>

#include <boost/utility/declval.hpp>

#include "blackhole/platform.hpp"

namespace blackhole {

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
            decltype(boost::declval<std::ostream&>() << boost::declval<T>())
        >::value
    >
{};

} // namespace supports

} // traits

} // namespace blackhole
