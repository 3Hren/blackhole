#pragma once

#include <vector>

#include "blackhole/config.hpp"

#include "blackhole/attribute/name.hpp"
#include "blackhole/attribute/value.hpp"
#include "blackhole/detail/config/platform/deprecated.hpp"

BLACKHOLE_BEG_NS

namespace attribute {

typedef std::pair<
    attribute::name_t,
    attribute_t
> pair_t;

typedef std::vector<
    pair_t
> set_t;

} // namespace attribute

namespace log {

typedef blackhole::attribute::pair_t attribute_pair_t
    BLACKHOLE_DEPRECATED("Use `attribute::pair_t` instead.");

typedef blackhole::attribute::set_t attributes_t
    BLACKHOLE_DEPRECATED("Use `attribute::set_t` instead.");

} // namespace log

BLACKHOLE_END_NS
