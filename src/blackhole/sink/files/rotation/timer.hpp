#pragma once

#include <ctime>

#include "blackhole/detail/config/nullptr.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

struct timepicker_t {
    std::time_t current() const {
        return std::time(nullptr);
    }
};

} // namespace rotation

} // namespace sink

} // namespace blackhole
