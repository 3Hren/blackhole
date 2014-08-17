#pragma once

#include <sstream>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

struct literal_t {
    const std::string literal;

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t&,
                    const attribute::set_view_t&) const {
        stream.rdbuf()->storage()->append(literal);
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
