#pragma once

#include "blackhole/config.hpp"

#include "blackhole/formatter/map/value.hpp"
#include "blackhole/repository/config/base.hpp"

BLACKHOLE_BEG_NS

class formatter_config_t : public repository::config::base_t {
public:
    mapping::value_t mapper;

    formatter_config_t(const std::string& type) :
        base_t(type)
    {}

    formatter_config_t(const std::string& type, const mapping::value_t& mapper) :
        base_t(type),
        mapper(mapper)
    {}
};

BLACKHOLE_END_NS
