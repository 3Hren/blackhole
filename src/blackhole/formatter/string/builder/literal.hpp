#pragma once

#include <sstream>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

struct literal_t {
    const std::string literal;

    void operator ()(std::ostringstream& stream, const mapping::value_t&, const log::attributes_t&) const {
        stream << literal;
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
