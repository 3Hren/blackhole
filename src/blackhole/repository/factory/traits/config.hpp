#pragma once

#include <string>

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

} // namespace blackhole
