#pragma once

#include <sstream>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

struct placeholder_t {
    const std::string placeholder;

    void operator ()(std::ostringstream& stream, const mapping::value_t& mapper, const log::attributes_t& attributes) const {
        auto it = attributes.find(placeholder);
        if (it == attributes.end()) {
            throw error_t("key '%s' was not provided", placeholder);
        }

        const log::attribute_value_t& value = it->second.value;
        mapper(stream, placeholder, value);
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
