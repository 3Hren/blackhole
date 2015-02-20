#pragma once

#include <ctime>

#include "blackhole/config.hpp"

#include "blackhole/detail/config/nullptr.hpp"

BLACKHOLE_BEG_NS

namespace sink {

namespace rotation {

struct timepicker_t {
    static std::time_t current() {
        return std::time(nullptr);
    }
};

} // namespace rotation

} // namespace sink

BLACKHOLE_END_NS
