#pragma once

#include <type_traits>

#include <boost/mpl/size.hpp>
#include <boost/mpl/unique.hpp>
#include <boost/mpl/vector.hpp>

namespace blackhole {

/// Metafunction, that determines whether a parameter pack has duplicate items.
template<typename... Args>
struct unique :
    public std::conditional<
        sizeof...(Args) == boost::mpl::size<
            typename boost::mpl::unique<
                boost::mpl::vector<Args...>,
                std::is_same<boost::mpl::_1, boost::mpl::_2>
            >::type
        >::value,
        std::true_type,
        std::false_type
    >::type
{};

} // namespace blackhole
