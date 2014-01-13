#pragma once

#include <string>
#include <vector>

#include "frontend.hpp"

namespace blackhole {

struct log_config_t {
    std::string name;
    std::vector<frontend_config_t> frontends;
};

} // namespace blackhole
