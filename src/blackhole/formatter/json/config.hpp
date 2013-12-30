#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace blackhole {

namespace formatter {

namespace json {

namespace map {

typedef std::unordered_map<std::string, std::string> naming_t;

struct positioning_t {
    typedef std::vector<std::string> positions_t;

    std::unordered_map<std::string, positions_t> specified;
    positions_t unspecified;
};

} // namespace map

struct config_t {
    bool newline;
    map::naming_t naming;
    map::positioning_t positioning;

    config_t() :
        newline(false)
    {}
};

} // namespace json

} // namespace formatter

} // namespace blackhole
