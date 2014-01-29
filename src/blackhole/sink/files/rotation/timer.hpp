#pragma once

#include <ctime>

namespace blackhole {

namespace sink {

struct timer_t {
    std::time_t current() const {
        return std::time(nullptr);
    }
};

} // namespace sink

} // namespace blackhole
