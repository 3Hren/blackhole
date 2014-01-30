#pragma once

#include <cctype>
#include <cstdint>
#include <string>

namespace blackhole {

namespace sink {

namespace rotation {

namespace naming {

inline bool all_digits(std::string::const_iterator& it, std::string::const_iterator end, int n) {
    for (; n > 0; --n) {
        if (it == end || !std::isdigit(*it++)) {
            return false;
        }
    }

    return true;
}

inline uint digits(uint number) {
    uint digits = 0;
    while (number) {
        number /= 10;
        digits++;
    }

    return digits;
}

} // namespace naming

} // namespace rotation

} // namespace sink

} // namespace blackhole
