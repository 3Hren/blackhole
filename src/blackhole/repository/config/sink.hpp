#pragma once

#include "blackhole/config.hpp"

#include "blackhole/repository/config/base.hpp"

BLACKHOLE_BEG_NS

class sink_config_t : public repository::config::base_t {
public:
    sink_config_t(const std::string& type) :
        base_t(type)
    {}
};

BLACKHOLE_END_NS
