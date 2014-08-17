#pragma once

#include <string>

#include "blackhole/platform/deprecated.hpp"

namespace blackhole {

namespace attribute {

typedef std::string name_t;

} // namespace attribute

typedef attribute::name_t attribute_name_t
    BLACKHOLE_DEPRECATED("Use `attribute::name_t` instead.");

} // namespace blackhole
