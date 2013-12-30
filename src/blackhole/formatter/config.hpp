#pragma once

#include <boost/any.hpp>

namespace blackhole {

template<typename T>
struct factory_traits {
    static typename T::config_type map_config(const boost::any& config);
};

} // namespace blackhole
