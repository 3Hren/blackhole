#pragma once

#include <string>

#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

class json_t {
public:
    std::string format(const log::record_t&) const {
        return std::string();
    }
};

} // namespace formatter

} // namespace blackhole
