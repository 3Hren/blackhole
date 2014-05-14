#pragma once

#include <string>

#include <memory>
#include <mutex>
#include <tuple>

#include "balancer.hpp"
#include "connection.hpp"
#include "log.hpp"
#include "pool.hpp"
#include "result.hpp"
#include "request/info.hpp"
#include "settings.hpp"

namespace elasticsearch {

void nullcb() {}

template<class Action> struct error_handler_t;
template<class Action> struct request_watcher_t;

class http_transport_t {
public:
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef boost::asio::io_service loop_type;
    typedef http_connection_t connection_type;
    typedef pool_t<connection_type> pool_type;
    typedef pool_type::endpoint_type endpoint_type;

    template<class Action> friend struct error_handler_t;
    template<class Action> friend struct request_watcher_t;

private:
    settings_t settings;

    loop_type& loop;
    logger_type& log;

    pool_type pool;
    std::unique_ptr<balancing::strategy<pool_type>> balancer;
    urlfetcher_t urlfetcher;
public:
    http_transport_t(settings_t settings, loop_type& loop, logger_type& log) :
        settings(settings),
        loop(loop),
        log(log),
        balancer(new balancing::round_robin<pool_type>()),
        urlfetcher(loop)
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
            endpoint, std::make_shared<connection_type>(endpoint, urlfetcher, log)
        );

        if (!inserted) {
            LOG(log, "adding endpoint to the pool is rejected - already exists");
            return;
        }
    }

    void remove_node(const endpoint_type& endpoint) {
        LOG(log, "removing '%s' from the pool ...", endpoint);
        pool.remove(endpoint);
    }

    void sniff() {
        sniff(&nullcb);
    }

    void sniff(std::function<void()>&& then) {
        LOG(log, "sniffing nodes info ...");
        auto callback = std::bind(
            &http_transport_t::on_sniff, this, std::placeholders::_1, then
        );
        perform(actions::nodes_info_t(), callback);
    }

    template<class Action>
    void perform(Action&& action,
                 typename callback<Action>::type callback,
                 int attempt = 1) {
        typedef typename Action::result_type result_type;

        BOOST_ASSERT(balancer);

        LOG(log, "performing '%s' request [attempt %d from %d] ...",
            action.name(), attempt, settings.retries);
        if (attempt > settings.retries) {
            LOG(log, "failed: too many attempts done");
            loop.post(
                std::bind(
                    callback,
                    result_type(error_t(generic_error_t("too many attempts")))
                )
            );
            return;
        }

        auto connection = balancer->next(pool);
        if (!connection) {
            LOG(log, "failed: no connections left");
            loop.post(
                std::bind(
                    callback,
                    result_type(error_t(generic_error_t("no connections left")))
                )
            );
            return;
        }

        LOG(log, "balancing at %s", connection->endpoint());
        request_watcher_t<Action> watcher {
            *this, action, callback, ++attempt
        };
        connection->perform(std::move(action), watcher);
    }

private:
    void on_sniff(result_t<response::nodes_info_t>::type&& result,
                  std::function<void()> next) {
        if (auto* nodes_info = boost::get<response::nodes_info_t>(&result)) {
            LOG(log, "successfully sniffed %d node(s)", nodes_info->nodes.size());

            const auto& nodes = nodes_info->nodes;
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            }
        }

        next();
    }
};

template<class Watcher>
struct error_handler_t : public boost::static_visitor<> {
    typedef Watcher watcher_type;

    const watcher_type& watcher;
    http_transport_t& transport;

    error_handler_t(const watcher_type& watcher, http_transport_t& transport) :
        watcher(watcher),
        transport(transport)
    {}

    void operator()(const connection_error_t& err) {
        LOG(transport.log, "request failed with error: %s", err.reason);

        transport.remove_node(err.endpoint);
        if (transport.settings.sniffer.when.error) {
            LOG(transport.log,
                "sniff.on.error is true - preparing to update nodes list");
            transport.sniff(watcher);
            return;
        }
    }

    void operator()(const generic_error_t& err) {
        LOG(transport.log, "request failed with error: %s", err.reason);
    }
};

template<class Action>
struct request_watcher_t {
    typedef Action action_type;
    typedef request_watcher_t<action_type> this_type;
    typedef typename action_type::result_type result_type;
    typedef typename callback<action_type>::type callback_type;

    http_transport_t& transport;
    const action_type action;
    const callback_type callback;
    const int attempt;

    template<class Result = result_type>
    typename std::enable_if<
        !std::is_same<Result, result_t<response::nodes_info_t>::type>::value
    >::type
    operator()(result_type&& result) const {
        if (error_t* error = boost::get<error_t>(&result)) {
            auto visitor = error_handler_t<this_type>(*this, transport);
            boost::apply_visitor(visitor, *error);
        }

        callback(std::move(result));
    }

    template<class Result = result_type>
    typename std::enable_if<
        std::is_same<Result, result_t<response::nodes_info_t>::type>::value
    >::type
    operator()(result_t<response::nodes_info_t>::type&& result) const {
        callback(std::move(result));
    }


    void operator()() {
        transport.perform(std::move(action), callback, attempt);
    }
};

} // namespace elasticsearch

