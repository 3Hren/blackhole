#pragma once

#include <unordered_map>

#include "blackhole/attribute/name.hpp"
#include "blackhole/attribute/value.hpp"
#include "blackhole/platform/deprecated.hpp"

namespace blackhole {

namespace attribute {

typedef std::pair<
    attribute::name_t,
    attribute_t
> pair_t;

typedef std::unordered_map<
    pair_t::first_type,
    pair_t::second_type
> set_t;

} // namespace attribute

namespace log {

typedef blackhole::attribute::pair_t attribute_pair_t
    BLACKHOLE_DEPRECATED("Use `attribute::pair_t` instead.");

typedef blackhole::attribute::set_t attributes_t
    BLACKHOLE_DEPRECATED("Use `attribute::set_t` instead.");

} // namespace log

} // namespace blackhole
