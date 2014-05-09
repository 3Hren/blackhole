#pragma once

#include <string>
#include <map>

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

} // namespace elasticsearch
