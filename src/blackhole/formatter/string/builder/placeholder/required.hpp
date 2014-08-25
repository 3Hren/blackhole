#pragma once

#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

namespace placeholder {

struct required_t {
    const std::string name;

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t& mapper,
                    const attribute::set_view_t& attributes) const
    {
        if (auto attribute = attributes.find(name)) {
            mapper(stream, name, attribute->value);
            return;
        }

        throw error_t("key '%s' was not provided", name);
    }
};

} // namespace placeholder

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
