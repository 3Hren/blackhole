#pragma once

#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/sink/syslog.hpp"
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

inline
void
map_severity(aux::attachable_ostringstream& stream, const severity& level) {
    static const char* describe[] = {
        "DEBUG",
        "NOTICE",
        "INFO",
        "WARN",
        "ERROR"
    };

    typedef blackhole::aux::underlying_type<severity>::type level_type;

    auto value = static_cast<level_type>(level);
    if(value < static_cast<level_type>(sizeof(describe) / sizeof(describe[0])) && value >= 0) {
        stream << describe[value];
    } else {
        stream << value;
    }
}

} // namespace defaults

namespace sink {

template<>
struct priority_traits<defaults::severity> {
    static inline
    priority_t map(defaults::severity lvl) {
        switch (lvl) {
        case defaults::severity::debug:
            return priority_t::debug;
        case defaults::severity::notice:
            return priority_t::info;
        case defaults::severity::info:
            return priority_t::info;
        case defaults::severity::warning:
            return priority_t::warning;
        case defaults::severity::error:
            return priority_t::err;
        }

        return priority_t::debug;
    }
};

} // namespace sink

} // namespace blackhole
