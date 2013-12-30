#pragma once

#include <boost/any.hpp>

namespace blackhole {

template<typename T>
struct factory_traits {
    static typename T::config_type map_config(const boost::any& config);
};

namespace aux {

template<typename T>
static void any_cast(const boost::any& from, T& to) {
    to = boost::any_cast<T>(from);
}

} // namespace aux

} // namespace blackhole
