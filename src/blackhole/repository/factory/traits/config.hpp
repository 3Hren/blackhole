#pragma once

#include <string>
#include <typeindex>

#include "blackhole/dynamic.hpp"

namespace blackhole {

template<typename T>
struct config_traits {
    //! \brief Statically maps sink or formatter type into unique key.
    /*! It contains information about its backends and strategies.
        For example: file sink with rotation is mapped into: `files/rotate`.
        Without rotation it will be just: `files`.
    */
    static std::string name() {
        return T::name();
    }
};

template<typename T>
struct match_traits {
    static bool matched(const std::string& type, const dynamic_t&) {
        return type == T::name();
    }

    static std::type_index ti(const std::string&, const dynamic_t&) {
        return typeid(T);
    }
};

} // namespace blackhole
