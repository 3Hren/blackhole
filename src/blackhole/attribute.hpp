#pragma once

#include <cstdint>
#include <initializer_list>
#include <unordered_map>

#include <boost/variant.hpp>

namespace blackhole {

namespace log {

typedef boost::variant<
    std::time_t,
    std::uint8_t,
    std::uint64_t,
    std::int64_t,
    std::double_t,
    std::string
> attribute_value_t;

typedef std::pair<
    std::string,
    attribute_value_t
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

} // namespace blackhole
