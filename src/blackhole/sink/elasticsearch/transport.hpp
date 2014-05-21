#pragma once

#include <string>

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <tuple>

#include "balancer.hpp"
#include "connection.hpp"
#include "log.hpp"
#include "pool.hpp"
#include "resolver.hpp"
#include "result.hpp"
#include "request/info.hpp"
#include "settings.hpp"

namespace elasticsearch {

namespace aux {

void empty() {}

} // namespace aux

template<class Action> struct error_handler_t;
template<class Action> struct request_watcher_t;

const std::string INET_ADDR_PREFIX = "inet[";
const std::string INET_ADDR_SUFFIX = "]";

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
            endpoint,
            std::make_shared<connection_type>(
                endpoint,
                settings.connections,
                urlfetcher,
                log
            )
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
        sniff(&aux::empty);
    }

    void sniff(std::function<void()>&& then) {
        LOG(log, "sniffing nodes info ...");
        auto callback = std::bind(
            &http_transport_t::on_sniff, this, std::placeholders::_1, then
        );
        perform(actions::nodes_info_t(), callback);
    }

    template<class Action>
    void perform(Action action,
                 typename callback<Action>::type callback,
                 int attempt = 1) {
        typedef typename Action::result_type result_type;

        BOOST_ASSERT(balancer);

        LOG(log, "performing '%s' request [attempt %d/%d] ...",
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
            if (attempt == 1) {
                add_nodes(settings.endpoints);
                loop.post(
                    std::bind(
                        &http_transport_t::perform<Action>,
                        this,
                        std::move(action),
                        callback,
                        ++attempt
                    )
                );
            } else {
                LOG(log, "failed: no connections left");
                loop.post(
                    std::bind(
                        callback,
                        result_type(error_t(generic_error_t("no connections left")))
                    )
                );
            }

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
        if (auto* info = boost::get<response::nodes_info_t>(&result)) {
            LOG(log, "successfully sniffed %d node%s",
                info->nodes.size(),
                info->nodes.size() > 1 ? "s" : ""
            );

            std::set<std::string> addresses;
            const auto& nodes = info->nodes;
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                const response::node_t& node = it->second;
                extract_addresses(addresses, node.addresses);
            }

            typedef boost::asio::ip::tcp protocol_type;
            std::vector<endpoint_type> endpoints;
            for (auto it = addresses.begin(); it != addresses.end(); ++it) {
                try {
                    endpoints.push_back(
                        resolver<protocol_type>::resolve(*it, loop)
                    );
                } catch (const std::exception& err) {
                    LOG(log, "failed to resolve %s address: %s", *it, err.what());
                }
            }

            add_nodes(endpoints);
        }

        next();
    }

    void extract_addresses(std::set<std::string>& result,
                           const response::node_t::addresses_type& addresses) const {
        auto it = addresses.find("http");
        if (it == addresses.end()) {
            return;
        }

        const std::string& address = it->second;
        if (boost::algorithm::starts_with(address, INET_ADDR_PREFIX)) {
            const std::string& parsed = address.substr(
                INET_ADDR_PREFIX.size(),
                address.size() -
                    INET_ADDR_PREFIX.size() -
                    INET_ADDR_SUFFIX.size()
            );
            result.insert(parsed);
        } else {
            LOG(log, "unknown address type: %s", address);
        }
    }
};

template<class Watcher>
struct error_handler_t : public boost::static_visitor<> {
    typedef Watcher watcher_type;

    const watcher_type& watcher;

    error_handler_t(const watcher_type& watcher) :
        watcher(watcher)
    {}

    void operator()(const connection_error_t& err) {
        http_transport_t& transport = watcher.transport;

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
        LOG(watcher.transport.log, "request failed with error: %s", err.reason);
    }
};

template<class Action>
struct request_watcher_t {
    typedef Action action_type;
    typedef request_watcher_t<action_type> this_type;
    typedef typename action_type::result_type result_type;
    typedef typename callback<action_type>::type callback_type;

    friend struct error_handler_t<request_watcher_t>;

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
            auto visitor = error_handler_t<this_type>(*this);
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

