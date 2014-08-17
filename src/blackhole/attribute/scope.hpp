#pragma once

#include <cstdint>

#include "blackhole/platform/deprecated.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace attribute {

enum class scope : std::uint8_t {
    local       = 1 << 0,   /* user-defined event attributes */
    event       = 1 << 1,   /* not user-defined event attributes, like timestamp or message */
    global      = 1 << 2,   /* logger object attributes */
    thread      = 1 << 3,   /* thread attributes */
    universe    = 1 << 4    /* singleton attributes for entire application */
};

typedef aux::underlying_type<scope>::type scope_underlying_type;

static const scope DEFAULT_SCOPE = scope::local;

} // namespace attribute

namespace log { namespace attribute {
typedef blackhole::attribute::scope scope BLACKHOLE_DEPRECATED("Use `attribute::scope_t` instead.");
} }

} // namespace blackhole
