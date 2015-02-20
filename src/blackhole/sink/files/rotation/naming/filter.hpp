#pragma once

#include <string>

#include "blackhole/config.hpp"

#include "filter/helpers.hpp"

BLACKHOLE_BEG_NS

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

BLACKHOLE_END_NS
