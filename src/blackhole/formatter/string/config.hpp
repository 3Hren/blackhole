#pragma once

#include <string>
#include <vector>

namespace blackhole {

namespace formatter {

namespace string {

struct config_t {
    std::string pattern;
    std::vector<std::string> attribute_names;
};

} // namespace string

} // namespace formatter

} // namespace blackhole
