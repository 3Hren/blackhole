#pragma once

#include <string>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace formatter {

namespace string {

struct config_t {
    std::string pattern;
    bool filter;

    config_t() : filter(true) {}
    explicit config_t(std::string pattern) :
        pattern(std::move(pattern)),
        filter(true)
    {}
};

} // namespace string

} // namespace formatter

BLACKHOLE_END_NS
