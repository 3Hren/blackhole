#pragma once

#include <tuple>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/attributes/convertible.hpp"
#include "blackhole/detail/traits/literal.hpp"
#include "blackhole/detail/traits/tuple.hpp"

namespace blackhole {

namespace aux {

//! Helper metafunction that determines if all even arguments is string literal.
//! It is used when determining if argument pack is emplace pack.
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

//! Helper metafunction that determines if all odd arguments is convertible to `attribute_value_t`.
//! It is used when determining if argument pack is emplace pack.
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

//! Helper metafunction that determines if argument pack is emplace pack.
//! Any argument pack is considered to be emplace pack if and only if:
//!  - it has even number of arguments;
//!  - all even arguments is string literal;
//!  - all odd arguments is convertible to `attribute_valut_t`.
template<class... Args>
struct is_emplace_pack {
    static const bool value =
            sizeof...(Args) % 2 == 0 &&
            all_first_string_literal<Args...>::value &&
            all_second_constructible<Args...>::value;
};

} // namespace aux

} // namespace blackhole
