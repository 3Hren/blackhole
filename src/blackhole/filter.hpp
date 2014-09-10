#pragma once

#include <functional>

#include "blackhole/forwards.hpp"

namespace blackhole {

typedef std::function<bool(const attribute::set_view_t& attributes)> filter_t;

namespace filter {

inline
__attribute__((always_inline))
bool none(const attribute::set_view_t&) {
    return true;
}

} // namespace filter

} // namespace blackhole
