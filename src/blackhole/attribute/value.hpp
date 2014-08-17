#pragma once

#include <cstdint>
#include <string>

#include <boost/variant.hpp>

namespace blackhole {

namespace attribute {

typedef boost::variant<
    std::int32_t,
    std::uint32_t,
    long,
    unsigned long,
    std::int64_t,
    std::uint64_t,
    std::double_t,
    std::string,
    timeval
> value_t;

} // namespace attribute

typedef attribute::value_t attribute_value_t;

} // namespace blackhole
