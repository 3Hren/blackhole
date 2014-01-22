#pragma once

#include <boost/any.hpp>

namespace blackhole {

template<typename T>
struct config_traits {
    static std::string name() {
        return T::name();
    }

    static std::string parse(const boost::any&) {
        return name();
    }
};

template<typename T>
struct factory_traits {
    static typename T::config_type map_config(const boost::any& config);
};

} // namespace blackhole
