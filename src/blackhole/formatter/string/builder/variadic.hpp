#pragma once

#include <sstream>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

static const char VARIADIC_KEY_PREFIX[] = "...";
static const std::size_t VARIADIC_KEY_PRFFIX_LENGTH = std::strlen(VARIADIC_KEY_PREFIX);

namespace builder {

struct variadic_t {
    typedef log::attribute::scope_underlying_type scope_underlying_type;

    const std::string placeholder;

    void operator ()(std::ostringstream& stream, const mapping::value_t&, const log::attributes_t& attributes) const {
        for (auto pit = placeholder.begin() + string::VARIADIC_KEY_PRFFIX_LENGTH; pit != placeholder.end(); ++pit) {
            const scope_underlying_type scope = *pit - '0';

            for (auto it = attributes.begin(); it != attributes.end();) {
                const std::string& name = it->first;
                const log::attribute_t& attribute = it->second;
                it++;
                if (static_cast<scope_underlying_type>(attribute.scope) & scope) {
                    //!@todo: Implement visitor to properly format int attributes (without quotes).
                    stream << "'" << name << "': '" << attribute.value << "'";
                    if (it != attributes.end()) {
                        stream << ", ";
                    }
                }
            }
        }
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
