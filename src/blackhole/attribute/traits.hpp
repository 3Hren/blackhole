#pragma once

#include <type_traits>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>

#include "value.hpp"

namespace blackhole {

namespace attribute {

/*!
 * Helper metafunction that checks if the type `T` is compatible with attribute
 * internal implementation, i.e. `attribute::value_t` variant can be constructed
 * using type `T`.
 * @note: This metafunction ignores implicit type conversion.
 */
template<typename T>
struct is_supported :
    public boost::mpl::contains<
        value_t::types,
        typename std::decay<T>::type
    >
{};

/*!
 * Helper metafunction that checks if `attribute::value_t` can be constructed
 * using type `T`.
 * @todo: I don't like it.
 */
template<typename T>
struct is_constructible {
    typedef boost::mpl::vector<
        const char*,    // Implicit literal to string conversion.
        char,
        unsigned char,
        short,
        unsigned short
    > additional_types;

    typedef typename std::conditional<
        boost::mpl::contains<
            additional_types,
            typename std::decay<T>::type
        >::value || is_supported<T>::value,
        std::true_type,
        std::false_type
    >::type type;

    static const bool value = type::value;
};

} // namespace attribute

} // namespace blackhole
