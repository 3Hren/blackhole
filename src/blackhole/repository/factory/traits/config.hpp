#pragma once

#include <memory>
#include <string>
#include <typeinfo>

#include "blackhole/config.hpp"

#include "blackhole/dynamic.hpp"

BLACKHOLE_BEG_NS

class type_index_t {
    const std::type_info* d;

public:
    type_index_t(const std::type_info& typeinfo) :
        d(&typeinfo)
    {}

    bool operator==(const type_index_t& other) const {
        return *d == *other.d;
    }

    size_t hash_code() const {
        static std::hash<std::string> hash;
        return hash(d->name());
    }
};

template<typename T>
struct match_traits {
    //! \brief Extract sink or formatter type index from its config.
    /*! Some sinks or formatters require more complex string mapping to type,
     * than provided by its static method `name`.
     * For example, `files_t` sink can be configured for using rotation, and
     * that rotation can have various strategies to determine when and how
     * actually do it.
     * In that case we can define unique keys explicitly in configuration file
     * (files/rotation/size) or to build that key from config. I'd preferred
     * the second way, because it is more agile.
     * So, suppose we have config:
     * "files": {
     *     "path": "test.log",
     *     "rotation": {
     *         "pattern": "test.log.%N",
     *         "backups": 5,
     *         "size": 1000000
     *     }
     * }
     *
     * If we find `rotation` key, then rotation is used, so we just return
     * type index with specified rotation strategy.
     *
     * By default returns Formatter or Sink typeid;
     */

    typedef type_index_t index_type;

    static type_index_t type_index(const std::string& type, const dynamic_t&) {
        if (type != T::name()) {
            return index_type(typeid(void));
        }

        return typeid(T);
    }
};

BLACKHOLE_END_NS

namespace std {

template<>
struct hash<blackhole::type_index_t> {
    size_t
    operator()(const blackhole::type_index_t& value) const BLACKHOLE_NOEXCEPT {
        return value.hash_code();
    }
};

} // namespace std
