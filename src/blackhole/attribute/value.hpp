#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <sys/time.h>

#include <boost/variant.hpp>

#include "blackhole/config.hpp"

#include "blackhole/detail/config/platform/deprecated.hpp"
#include "blackhole/detail/config/underlying.hpp"

template<class Char, class Traits>
inline
std::basic_ostream<Char, Traits>&
operator<<(std::basic_ostream<Char, Traits>& stream, const timeval& tv) {
    stream << tv.tv_sec << "." << tv.tv_usec;
    return stream;
}

inline
bool
operator==(const timeval& lhs, const timeval& rhs) {
    return lhs.tv_sec == rhs.tv_sec && lhs.tv_usec == rhs.tv_usec;
}

BLACKHOLE_BEG_NS

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

struct attribute_t {
    attribute::value_t value;

    attribute_t() :
        value(std::uint8_t(0))
    {}

    attribute_t(const char* value) :
        value(value)
    {}

    attribute_t(std::string value) :
        value(std::move(value))
    {}

    attribute_t(const attribute::value_t& value) :
        value(value)
    {}

    attribute_t(attribute::value_t&& value) :
        value(std::move(value))
    {}

    // Force compiler to keep enum values according to its underlying type.
    // It is needed, cause for weakly-typed enums its underlying type and its values underlying
    // types may vary, which leads to exception while extracting from variant.
    template<typename T, class = typename std::enable_if<std::is_enum<T>::value>::type>
    attribute_t(T value) :
        value(static_cast<typename aux::underlying_type<T>::type>(value))
    {}

    bool operator==(const attribute_t& other) const {
        return value == other.value;
    }
};

namespace log {

typedef blackhole::attribute::value_t attribute_value_t
    BLACKHOLE_DEPRECATED("Use `attribute::value_t` instead.");

typedef blackhole::attribute_t attribute_t
    BLACKHOLE_DEPRECATED("Use `blackhole::attribute_t` instead.");

} // namespace log

BLACKHOLE_END_NS
