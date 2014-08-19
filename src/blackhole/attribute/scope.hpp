#pragma once

#include <cstdint>

#include "blackhole/detail/config/platform/deprecated.hpp"
#include "blackhole/detail/config/underlying.hpp"

namespace blackhole {

namespace attribute {

//! @deprecated: It's very likely, that attribute scope will be dropped soon.
enum class scope_t : std::uint8_t {
    local       = 1 << 0,   /* user-defined event attributes */
    event       = 1 << 1,   /* not user-defined event attributes, like timestamp or message */
    global      = 1 << 2,   /* logger object attributes */
    thread      = 1 << 3,   /* thread attributes */
    universe    = 1 << 4    /* singleton attributes for entire application */
};

typedef aux::underlying_type<scope_t>::type scope_underlying_type;

static const scope_t DEFAULT_SCOPE = scope_t::local;

} // namespace attribute

namespace log {

namespace attribute {

typedef blackhole::attribute::scope_t scope
    BLACKHOLE_DEPRECATED("Use `attribute::scope_t` instead.");

} // namespace attribute

} // namespace log

} // namespace blackhole
