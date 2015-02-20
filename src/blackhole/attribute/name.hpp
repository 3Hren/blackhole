#pragma once

#include <string>

#include "blackhole/config.hpp"

#include "blackhole/detail/config/platform/deprecated.hpp"

BLACKHOLE_BEG_NS

namespace attribute {

typedef std::string name_t;

} // namespace attribute

typedef blackhole::attribute::name_t attribute_name_t
    BLACKHOLE_DEPRECATED("Use `attribute::name_t` instead.");

BLACKHOLE_END_NS
