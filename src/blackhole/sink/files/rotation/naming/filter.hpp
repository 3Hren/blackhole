#pragma once

#include <string>

#include "filter/helpers.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

namespace naming {

struct filter_t {
    const std::string& pattern;

    filter_t(const std::string& pattern) :
        pattern(pattern)
    {}

    bool operator ()(const std::string& filename) const {
        return !aux::matched(pattern, filename);
    }
};

} // namespace naming

} // namespace rotation

} // namespace sink

} // namespace blackhole
