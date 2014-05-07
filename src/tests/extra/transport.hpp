#pragma once

#include <memory>
#include <mutex>

#include "balancing.hpp"
#include "log.hpp"
#include "pool.hpp"

namespace elasticsearch {

class http_connection_t {
public:
    typedef boost::asio::io_service loop_type;
    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::endpoint endpoint_type;

public:
    http_connection_t(endpoint_type, loop_type&)
    {}
};

class http_transport_t {
public:
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef boost::asio::io_service loop_type;
    typedef http_connection_t connection_type;
    typedef pool_t<connection_type> pool_type;
    typedef pool_type::endpoint_type endpoint_type;

private:
    loop_type& loop;
    logger_type& log;

    pool_type pool;
    std::unique_ptr<balancing::strategy<pool_type>> balancer;

    mutable std::mutex mutex;

public:
    http_transport_t(loop_type& loop, logger_type& log) :
        loop(loop),
        log(log)
    {}

    void add_nodes(const std::vector<endpoint_type>& endpoints) {
        for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
            add_node(*it);
        }
    }

    void add_node(const endpoint_type& endpoint) {
        boost::system::error_code ec;
        auto address = endpoint.address().to_string(ec);
        if (ec) {
            //!@todo: Do something useful.
        }

        LOG(log, "adding '%s:%d' to the pool", address, endpoint.port());

        std::lock_guard<std::mutex> lock(mutex);
        if (pool.contains(endpoint)) {
            LOG(log, "adding endpoint to the pool is rejected - already exists");
            return;
        }
        pool.insert(endpoint, std::make_shared<connection_type>(endpoint, loop));
    }

    void sniff() {}
};

} // namespace elasticsearch
