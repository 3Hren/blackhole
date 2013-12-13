#pragma once

#include "attribute.hpp"

namespace blackhole {

namespace log {

struct record_t {
    attributes_t attributes;

    bool valid() const {
        return !attributes.empty();
    }

    template<typename T>
    inline T extract(const std::string& name) const {
        return blackhole::attribute::traits<T>::extract(attributes, name);
    }
};

} // namespace log

} // namespace blackhole
