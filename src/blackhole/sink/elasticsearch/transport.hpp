#pragma once

#include <memory>
#include <mutex>
#include <tuple>

#include "balancer.hpp"
#include "connection.hpp"
#include "log.hpp"
#include "pool.hpp"
#include "result.hpp"
#include "request/info.hpp"

namespace elasticsearch {

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
public:
    http_transport_t(loop_type& loop, logger_type& log) :
        loop(loop),
        log(log),
        balancer(new balancing::round_robin<pool_type>())
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
            LOG(log, "can't add node endpoint '%s': %s", endpoint, ec.message());
            return;
        }

        LOG(log, "adding '%s:%d' to the pool ...", address, endpoint.port());
        bool inserted;
        std::tie(std::ignore, inserted) = pool.insert(
            endpoint, std::make_shared<connection_type>(endpoint, loop)
        );

        if (!inserted) {
            LOG(log, "adding endpoint to the pool is rejected - already exists");
            return;
        }
    }

    void sniff() {
        LOG(log, "sniffing nodes info ...");
        perform(actions::nodes_info_t(),
                std::bind(&http_transport_t::on_sniff, this, std::placeholders::_1));
    }

    template<class Action>
    void perform(Action&& action, typename callback<Action>::type callback) {
        BOOST_ASSERT(balancer);

        LOG(log, "performing request ...");
        auto connection = balancer->next(pool);
        if (!connection) {
            //!@todo: Write error to the callback via post.
            return;
        }

        LOG(log, "balancing at %s", connection->endpoint());
        connection->perform(std::move(action), callback);
    }

private:
    void on_sniff(result_t<response::nodes_info_t>&&) {}
};

} // namespace elasticsearch

