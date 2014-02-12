#pragma once

#include <boost/any.hpp>

namespace blackhole {

namespace generator {

template<typename T>
struct id {
    //! Extract sink or formatter unique key from its config.
    // Some sinks or formatters require more complex string mapping to type, than provided by its
    // statis method `name`.
    // For example, `files_t` sink can be configured for using rotation, and that rotation
    // can have various strategies to determine when and how actually do it.
    // In that case we can define unique keys explicitly in configuration file (files/rotation/size)
    // or to build that key from config. I'd preferred the second way, because it is more agile.
    // So, suppose we have config:
    // "files" : {
    //      "path": "test.log",
    //      "rotation": {
    //          "pattern": "test.log.%N",
    //          "backups": 5,
    //          "size": 1000000
    //      }
    //  }

    // If we find `rotation` key, then rotation is used, so we add `/rotation` key to
    // the `files`. After determining its strategy (size) we also append `/size` and
    // the unique key (files/rotation/size) is completely built.
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
     * Without rotation it will be just: `files`.
     */
    static std::string name() {
        return T::name();
    }
};

template<class T>
struct factory_traits {
    static_assert(sizeof(T) == -1,
                  "You should create 'factory_traits' template specialization for "
                  "your type to properly map generic config object on real config.");
};

} // namespace blackhole
