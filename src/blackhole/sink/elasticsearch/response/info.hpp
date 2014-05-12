#pragma once

#include <string>
#include <map>

#include "extract.hpp"

namespace elasticsearch {

namespace response {

struct node_t {
    std::map<std::string, std::string> addresses;
};

struct nodes_info_t {
    std::string cluster_name;
    std::map<std::string, node_t> nodes;
};

} // namespace response

template<>
struct extractor_t<response::nodes_info_t> {
    static response::nodes_info_t extract(const rapidjson::Value&) {
        return response::nodes_info_t();
    }
};

} // namespace elasticsearch
