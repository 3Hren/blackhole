#pragma once

#include <sstream>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

struct placeholder_t {
    const std::string placeholder;

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t& mapper,
                    const attribute::set_view_t& attributes) const
    {
        if (auto attribute = attributes.find(placeholder)) {
            mapper(stream, placeholder, attribute->value);
            return;
        }

        throw error_t("key '%s' was not provided", placeholder);
    }
};

struct optional_placeholder_t {
    const std::string placeholder;

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t& mapper,
                    const attribute::set_view_t& attributes) const
    {
        if (auto attribute = attributes.find(placeholder)) {
            mapper(stream, placeholder, attribute->value);
            return;
        } else {
        }
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
