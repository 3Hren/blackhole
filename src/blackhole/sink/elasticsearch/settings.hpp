#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace elasticsearch {

namespace defaults {

typedef boost::asio::ip::tcp::endpoint endpoint_type;

const endpoint_type endpoint(boost::asio::ip::address_v4(), 9200);

} // namespace defaults

struct settings_t {
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

    std::string index;
    std::vector<endpoint_type> endpoints;

    struct sniffer_t {
        struct {
            bool start;
            bool error;
        } when;

        long timeout;
        long invertal;
    } sniffer;

    int connections;
    int retries;
    long timeout;


    settings_t() :
        index("log"),
        endpoints(std::vector<endpoint_type>({ defaults::endpoint })),
        sniffer({{ true, true }, 10, 60000 }),
        connections(20),
        retries(3),
        timeout(1000)
    {}
};

} // namespace elasticsearch
