#pragma once

#include <string>
#include <vector>

#include "blackhole/config.hpp"

#include "frontend.hpp"

BLACKHOLE_BEG_NS

struct log_config_t {
    std::string name;
    std::vector<frontend_config_t> frontends;
};

BLACKHOLE_END_NS
