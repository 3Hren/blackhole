#pragma once

#include "blackhole/formatter/map/value.hpp"
#include "blackhole/repository/config/base.hpp"

namespace blackhole {

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

} // namespace blackhole
