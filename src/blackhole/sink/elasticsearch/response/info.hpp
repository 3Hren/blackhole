#pragma once

#include <string>
#include <map>

#include "extract.hpp"

namespace elasticsearch {

namespace response {

struct node_t {
    typedef std::map<std::string, std::string> addresses_type;

    addresses_type addresses;
};

struct nodes_info_t {
    std::string cluster_name;
    std::map<std::string, node_t> nodes;
};

} // namespace response

template<>
struct extractor_t<response::node_t> {
    static response::node_t extract(const rapidjson::Value& object) {
        aux::extract_helper_t extract(object);
        response::node_t node;
        extract.find(
            filter::endswith_t { "_address" },
            mapped::substring_t { "_address" },
            node.addresses
        );
        return node;
    }
};

template<>
struct extractor_t<response::nodes_info_t> {
    static response::nodes_info_t extract(const rapidjson::Value& object) {
        aux::extract_helper_t extract(object);
        response::nodes_info_t nodes_info;
        extract.to("cluster_name", nodes_info.cluster_name);
        extract.to("nodes", nodes_info.nodes);
        return nodes_info;
    }
};

} // namespace elasticsearch
