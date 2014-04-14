#pragma once

#include "attribute.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace log {

struct record_t {
    attributes_t attributes;

    operator bool() const {
        return !attributes.empty();
    }

    bool valid() const {
        return !attributes.empty();
    }

    template<typename T, typename... Args>
    inline void fill(T pair, Args&&... args) {
        attributes.insert(pair);
        fill(std::forward<Args>(args)...);
    }

    inline void fill() {}

    template<typename T>
    inline T extract(const std::string& name) const {
        return blackhole::attribute::traits<T>::extract(attributes, name);
    }
};

} // namespace log

} // namespace blackhole
