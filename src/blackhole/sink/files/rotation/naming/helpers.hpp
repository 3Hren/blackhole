#pragma once

#include <cctype>
#include <cstdint>
#include <string>

namespace blackhole {

namespace sink {

namespace matching {

inline bool all_digits(std::string::const_iterator& it, std::string::const_iterator end, int n) {
    for (; n > 0; --n) {
        const char c = *it;
        if (!std::isdigit(c) || it == end) {
            return false;
        }

        it++;
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

} // namespace matching

} // namespace sink

} // namespace blackhole
