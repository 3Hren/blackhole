#pragma once

#include <type_traits>

#include <boost/mpl/size.hpp>
#include <boost/mpl/unique.hpp>
#include <boost/mpl/vector.hpp>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

/// General definition of the helper class
template<typename... Args> struct from_variadic;

/// This one handles the case when no types passed.
template<>
struct from_variadic<> {
    typedef boost::mpl::vector<> type;
};

/// This one is a specialization for the case when only one type is passed.
template<typename T>
struct from_variadic<T> {
    typedef boost::mpl::vector<T> type;
};

/*!
 * This specialization does the actual job.
 * It splits the whole pack into 2 parts: one with single type T and the rest
 * of types Args.
 * As soon as it is done T is added to an mpl::vector.
 */
template<typename T, typename... Args>
struct from_variadic<T, Args...> {
    typedef typename boost::mpl::push_front<
        typename from_variadic<Args...>::type,
        T
    >::type type;
};

/// Metafunction, that detects whether a parameter pack has duplicate items.
template<typename... Args>
struct unique :
    public std::conditional<
        sizeof...(Args) == boost::mpl::size<
            typename boost::mpl::unique<
                typename from_variadic<Args...>::type,
                std::is_same<boost::mpl::_1, boost::mpl::_2>
            >::type
        >::value,
        std::true_type,
        std::false_type
    >::type
{};

BLACKHOLE_END_NS
