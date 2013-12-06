#pragma once

#include <cstdint>
#include <initializer_list>
#include <unordered_map>

#include <boost/variant.hpp>

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

enum class scope {
    local,      /* user-defined event attributes*/
    event,      /* not user-defined event attributes, like timestamp or message */
    global,     /* logger object attributes*/
    thread,     /* thread attributes */
    universe    /* singleton attributes for entire application */
};

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

} // namespace attr

} // namespace blackhole
