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
    std::string type;
    std::vector<endpoint_type> endpoints;

    struct sniffer_t {
        struct {
            bool start;
            bool error;
        } when;

        std::uint64_t invertal;
    } sniffer;

    int connections;
    int retries;
    std::uint64_t timeout;

    settings_t() :
        index("logs"),
        type("log"),
        endpoints(std::vector<endpoint_type>({ defaults::endpoint })),
        sniffer({{ true, true }, 60000 }),
        connections(20),
        retries(3),
        timeout(1000)
    {}
};

} // namespace elasticsearch

