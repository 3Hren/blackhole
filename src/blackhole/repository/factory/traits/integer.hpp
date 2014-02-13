#pragma once

#include <cstdint>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>

namespace blackhole {

typedef boost::mpl::vector<
    std::uint8_t,
    std::uint16_t,
    std::uint32_t,
    std::uint64_t,
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t
> supported_integers_t;

template<typename T>
struct is_supported_integer {
    static const bool value = boost::mpl::contains<supported_integers_t, T>::value;
};

} // namespace blackhole
