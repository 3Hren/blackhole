#pragma once

#include <tuple>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/attributes/convertible.hpp"
#include "blackhole/detail/traits/literal.hpp"
#include "blackhole/detail/traits/tuple.hpp"

namespace blackhole {

namespace aux {

//!@todo: Need comments!
template<class... Args>
struct all_first_string_literal {
    static const bool value = tuple::all<
        typename tuple::map<
            is_string_literal_type,
            typename tuple::remove_index<
                typename tuple::filter<
                    tuple::slice<0, -1, 2>::type,
                    typename tuple::add_index<
                        std::tuple<Args...>
                    >::type
                >::type
            >::type
        >::type
    >::value;
};

//!@todo: Need comments!
template<class... Args>
struct all_second_constructible {
    static const bool value = tuple::all<
        typename tuple::map<
            is_convertible,
            typename tuple::remove_index<
                typename tuple::filter<
                    tuple::slice<1, -1, 2>::type,
                    typename tuple::add_index<
                        std::tuple<Args...>
                    >::type
                >::type
            >::type
        >::type
    >::value;
};

//!@todo: Need comments!
template<class... Args>
struct is_emplace_pack {
    static const bool value =
            sizeof...(Args) % 2 == 0 &&
            all_first_string_literal<Args...>::value &&
            all_second_constructible<Args...>::value;
};

} // namespace aux

} // namespace blackhole
