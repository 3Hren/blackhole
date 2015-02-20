#pragma once

#include <functional>

#include "blackhole/config.hpp"

#include "blackhole/detail/config/inline.hpp"
#include "blackhole/forwards.hpp"

BLACKHOLE_BEG_NS

typedef std::function<bool(const attribute::combined_view_t& attributes)> filter_t;

namespace filter {

inline
BLACKHOLE_ALWAYS_INLINE
bool none(const attribute::combined_view_t&) {
    return true;
}

} // namespace filter

BLACKHOLE_END_NS
