#pragma once

#include <cstdint>
#include <string>

#include <boost/variant.hpp>

#include "blackhole/attribute/scope.hpp"
#include "blackhole/platform/deprecated.hpp"

namespace blackhole {

namespace attribute {

typedef boost::variant<
    std::int32_t,
    std::uint32_t,
    long,
    unsigned long,
    std::int64_t,
    std::uint64_t,
    double,
    std::string,
    timeval
> value_t;

} // namespace attribute

typedef attribute::value_t attribute_value_t BLACKHOLE_DEPRECATED("Use `attribute::value_t` instead.");

//!@todo: Now I keep multiple attribute sets in single view class. Seems, there
//!       is no need for this class anymore, cause attribute groups can be
//!       received from view class explicitly (without that freaking iterate
//!       over ALL attributes).
struct attribute_t {
    attribute::value_t value;
    attribute::scope scope;

    attribute_t() :
        value(std::uint8_t(0)),
        scope(attribute::DEFAULT_SCOPE)
    {}

    attribute_t(const attribute::value_t& value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(value),
        scope(type)
    {}

    attribute_t(attribute::value_t&& value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(std::move(value)),
        scope(type)
    {}

    // Force compiler to keep enum values according to its underlying type.
    // It is needed, cause for weakly-typed enums its underlying type and its values underlying
    // types may vary, which leads to exception while extracting from variant.
    template<typename T, class = typename std::enable_if<std::is_enum<T>::value>::type>
    attribute_t(T value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(static_cast<typename aux::underlying_type<T>::type>(value)),
        scope(type)
    {}

    bool operator==(const attribute_t& other) const {
        return value == other.value && scope == other.scope;
    }
};

} // namespace blackhole
