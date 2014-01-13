#pragma once

#include <string>

#include <boost/any.hpp>

#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

struct formatter_config_t {
    std::string type;
    boost::any config;
    mapping::value_t mapper;

    formatter_config_t() {}

    formatter_config_t(const std::string& type, const boost::any& config) :
        type(type),
        config(config)
    {}

    formatter_config_t(const std::string& type, const boost::any& config, const mapping::value_t& mapper) :
        type(type),
        config(config),
        mapper(mapper)
    {}
};

} // namespace blackhole
