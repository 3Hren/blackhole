#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

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
    class builder_t;
    class factory_t;
    std::unique_ptr<factory_t> factory;

public:
    json_t();
    json_t(routing_t routing);
    json_t(routing_t routing, mapping_t mapping);

    ~json_t();

    auto format(const record_t& record, writer_t& writer) -> void;
};

}  // namespace formatter
}  // namespace blackhole
