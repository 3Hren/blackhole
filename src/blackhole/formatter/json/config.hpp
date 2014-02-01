#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace blackhole {

namespace formatter {

namespace json {

namespace map {

typedef std::unordered_map<std::string, std::string> mapping_t;

struct routing_t {
    typedef std::vector<std::string> routes_t;

    std::unordered_map<std::string, routes_t> specified;
    routes_t unspecified;
};

} // namespace map

struct config_t {
    bool newline;
    map::mapping_t naming;
    map::routing_t routing;

    config_t() :
        newline(false)
    {}
};

} // namespace json

} // namespace formatter

} // namespace blackhole
