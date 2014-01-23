#pragma once

#include <boost/any.hpp>

namespace blackhole {

namespace generator {

template<typename T>
struct id {
    //! \brief Extract sink or formatter unique key from its config.
    static std::string extract(const boost::any&) {
        return T::name();
    }
};

} // namespace generator

template<typename T>
struct config_traits {
    //! \brief Statically maps sink or formatter type into unique key.
    /*! It contains information about its backends and strategies.
     * For example: file sink with rotation is mapped into: `files/rotate`.
     * Without rotation it wile be just: `files`.
     */
    static std::string name() {
        return T::name();
    }
};

template<typename T>
struct factory_traits {
    static typename T::config_type map_config(const boost::any& config);
};

} // namespace blackhole
