#pragma once

#include <string>

#include <boost/any.hpp>

namespace blackhole {

template<typename T>
struct unique_id_traits {
    //! \brief Extract sink or formatter unique key from its config.
    /*! Some sinks or formatters require more complex string mapping to type, than provided by its
        statis method `name`.
        For example, `files_t` sink can be configured for using rotation, and that rotation
        can have various strategies to determine when and how actually do it.
        In that case we can define unique keys explicitly in configuration file (files/rotation/size)
        or to build that key from config. I'd preferred the second way, because it is more agile.
        So, suppose we have config:
        "files": {
            "path": "test.log",
            "rotation": {
                "pattern": "test.log.%N",
                "backups": 5,
                "size": 1000000
            }
        }

        If we find `rotation` key, then rotation is used, so we add `/rotation` key to
        the `files`. After determining its strategy (size) we also append `/size` and
        the unique key (files/rotation/size) is completely built.

        By default returns Formatter or Sink name.
    */
    template<class Dynamic>
    static std::string generate(const Dynamic&) {
        return T::name();
    }
};

} // namespace blackhole
