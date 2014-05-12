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
        int retries;
        struct {
            bool start;
            bool error;
        } when;

        long timeout;
        long invertal;
    } sniffer;

    struct limits_t {
        int connections;
        long timeout;
    } limits;


    settings_t() :
        index("log"),
        endpoints(std::vector<endpoint_type>({ defaults::endpoint })),
        sniffer({ 3, { true, true }, 10, 60000 }),
        limits({ 20, 30000 })
    {}
};

} // namespace elasticsearch
