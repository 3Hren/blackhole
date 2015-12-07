#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/pointer.h>

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

class route_t {
public:
    rapidjson::Pointer pointer;

    route_t(const std::string& source) : pointer(source) {}
    route_t(rapidjson::Pointer pointer) : pointer(pointer) {}

    auto append(const std::string& name) const -> route_t {
        return {pointer.Append(name)};
    }
};

struct routing_t {
    typedef std::map<std::string, std::vector<std::string>> specified_type;
    typedef std::string unspecified_type;

    specified_type specified;
    unspecified_type unspecified;

    routing_t(unspecified_type unspecified = "") :
        unspecified(std::move(unspecified))
    {}

    auto spec(std::string route, std::vector<std::string> attributes) -> routing_t& {
        specified[std::move(route)] = std::move(attributes);
        return *this;
    }
};

typedef std::unordered_map<std::string, std::string> mapping_t;

// TODO: Add severity mapping support.
// TODO: Add timestamp mapping support.
// TODO: Take a doc from site.
class json_t : public formatter_t {
    std::unique_ptr<route_t> base;

    /// Mapping from attribute name to its route.
    std::unordered_map<std::string, route_t> routing;

    /// Mapping from attribute name to its key.
    mapping_t mapping;

public:
    json_t();
    json_t(routing_t routing);
    json_t(routing_t routing, mapping_t mapping);

    ~json_t();

    auto format(const record_t& record, writer_t& writer) -> void;
};

}  // namespace formatter
}  // namespace blackhole
