#pragma once

#include <string>

#include <boost/any.hpp>

namespace blackhole {

struct sink_config_t {
    std::string type;
    boost::any config;
};

} // namespace blackhole
