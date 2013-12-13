#pragma once

#include <cstdint>
#include <initializer_list>
#include <unordered_map>

#include <boost/variant.hpp>

#include "utils/underlying.hpp"

namespace blackhole {

namespace log {

typedef boost::variant<
    std::uint8_t,
    std::int32_t,
    std::uint64_t,
    std::int64_t,
    std::double_t,
    std::time_t,
    std::string
> attribute_value_t;

namespace attribute {

enum class scope : std::uint8_t {
    local = 1,      /* user-defined event attributes*/
    event = 2,      /* not user-defined event attributes, like timestamp or message */
    global = 4,     /* logger object attributes*/
    thread = 8,     /* thread attributes */
    universe = 16   /* singleton attributes for entire application */
};

typedef typename aux::underlying_type<scope>::type scope_underlying_type;

static const scope DEFAULT_SCOPE = scope::local;

} // namespace attribute

struct attribute_t {
    attribute_value_t value;
    attribute::scope scope;

    attribute_t() :
        value(std::uint8_t(0)),
        scope(attribute::DEFAULT_SCOPE)
    {}

    attribute_t(const attribute_value_t& value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(value),
        scope(type)
    {}
};

typedef std::pair<
    std::string,
    attribute_t
> attribute_pair_t;

typedef std::unordered_map<
    attribute_pair_t::first_type,
    attribute_pair_t::second_type
> attributes_t;

} // namespace log

inline log::attributes_t merge(const std::initializer_list<log::attributes_t>& args) {
    log::attributes_t summary;
    for (auto it = args.begin(); it != args.end(); ++it) {
        summary.insert(it->begin(), it->end());
    }

    return summary;
}

namespace attribute {

// Dynamic attribute factory function.
template<typename T>
inline log::attribute_pair_t make(const std::string& name, const T& value) {
    return std::make_pair(name, log::attribute_t(value));
}

// Attribute packing/unpacking/extracting.
template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename aux::underlying_type<T>::type underlying_type;

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name).value));
    }
};

} // namespace attribute

} // namespace blackhole
