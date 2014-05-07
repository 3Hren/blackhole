#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace elasticsearch {

struct settings_t {
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

    std::string index;
    std::vector<endpoint_type> endpoints;
    struct {
        struct {
            bool start;
        } when;
    } sniffer;

    settings_t() :
        index("log"),
        endpoints(std::vector<endpoint_type>({
            { endpoint_type(boost::asio::ip::address_v4(), 9200) }
        })),
        sniffer({ { true } })
    {}
};

} // namespace elasticsearch
