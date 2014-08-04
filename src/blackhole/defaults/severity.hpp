#pragma once

#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace defaults {

enum class severity {
    debug,
    notice,
    info,
    warning,
    error
};

void
map_severity(aux::attachable_ostringstream& stream, const severity& level) {
    static const char* describe[] = {
        "DEBUG",
        "NOTICE",
        "INFO",
        "WARNING",
        "ERROR"
    };

    typedef blackhole::aux::underlying_type<severity>::type level_type;

    auto value = static_cast<level_type>(level);
    if(value < static_cast<level_type>(sizeof(describe) / sizeof(describe[0])) && value > 0) {
        stream << describe[value];
    } else {
        stream << value;
    }
}

} // namespace defaults

} // namespace blackhole
