#pragma once

#include "blackhole/repository/config/base.hpp"

namespace blackhole {

class sink_config_t : public repository::config::base_t {
public:
    sink_config_t(const std::string& type) :
        base_t(type)
    {}
};

} // namespace blackhole
