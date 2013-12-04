#pragma once

#include "attribute.hpp"

namespace blackhole {

namespace log {

struct record_t {
    attributes_t attributes;

    bool valid() const {
        return !attributes.empty();
    }
};

} // namespace log

} // namespace blackhole
