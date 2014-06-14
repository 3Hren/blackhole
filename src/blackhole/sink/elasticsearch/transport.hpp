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

inline void empty() {}

} // namespace aux

const std::string INET_ADDR_PREFIX = "inet[";
const std::string INET_ADDR_SUFFIX = "]";

//! @threadsafe: All methods are thread-safe.
//! @note: This class must live until event loop completely stopped.
template<
    class Connection = http_connection_t,
    class Pool = pool_t<Connection>
>
class http_transport_t {
public:
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef boost::asio::io_service loop_type;
    typedef Connection connection_type;
    typedef Pool pool_type;
    typedef typename pool_type::endpoint_type endpoint_type;

protected:
    const settings_t settings;

    loop_type& loop;
    boost::asio::deadline_timer timer;
    boost::posix_time::milliseconds interval;
    std::atomic<bool> active;

    logger_type& log;

    pool_type pool;
    std::unique_ptr<balancing::strategy<pool_type>> balancer;

public:
    http_transport_t(settings_t settings, loop_type& loop, logger_type& log) :
        settings(settings),
        loop(loop),
        timer(loop),
        interval(settings.sniffer.invertal),
        active(true),
        log(log),
        balancer(new balancing::round_robin<pool_type>())
    {
        sniff(interval);
    }

    void cancel() {
        active = false;
        timer.cancel();
    }

    void add_nodes(const std::vector<endpoint_type>& endpoints) {
        for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
            add_node(*it);
        }
    }

    void add_node(const endpoint_type& endpoint) {
        boost::system::error_code ec;
        auto address = endpoint.address().to_string(ec);
        if (ec) {
            ES_LOG(log, "can't add node endpoint '%s': %s", endpoint, ec.message());
            return;
        }

        ES_LOG(log, "adding '%s:%d' to the pool ...", address, endpoint.port());
        bool inserted;
        std::tie(std::ignore, inserted) = pool.insert(
            endpoint,
            std::make_shared<connection_type>(
                endpoint,
                loop,
                log
            )
        );

        if (!inserted) {
            ES_LOG(log, "adding endpoint to the pool is rejected - already exists");
        }
    }

    void remove_node(const endpoint_type& endpoint) {
        ES_LOG(log, "removing '%s' from the pool ...", endpoint);
        pool.remove(endpoint);
    }

    void sniff() {
        sniff(&aux::empty);
    }

    void sniff(std::function<void()>&& then) {
        ES_LOG(log, "sniffing nodes info ...");
        sniff(interval);
        auto callback = std::bind(
            &http_transport_t::on_sniff, this, std::placeholders::_1, then
        );
        perform(actions::nodes_info_t(), callback);
    }

    void sniff(boost::posix_time::milliseconds interval) {
        if (!active) {
            ES_LOG(log, "stop sniffing on timer: transport no longer active");
            return;
        }

        timer.expires_from_now(interval);
        timer.async_wait(
            std::bind(
                &http_transport_t::on_sniff_timer,
                this,
                std::placeholders::_1
            )
        );
    }

    template<class Action>
    void perform(Action action,
                 typename callback<Action>::type callback,
                 int attempt = 1) {
        typedef typename Action::result_type result_type;

        BOOST_ASSERT(balancer);

        ES_LOG(log, "performing '%s' request [attempt %d/%d] ...",
            action.name(), attempt, settings.retries);
        if (attempt > settings.retries) {
            ES_LOG(log, "failed: too many attempts done");
            loop.post(
                std::bind(
                    callback,
                    result_type(error_t(generic_error_t("too many attempts")))
                )
            );
            return;
        }

        if (auto connection = balancer->next(pool)) {
            ES_LOG(log, "balancing at %s", connection->endpoint());
            auto watcher = std::bind(
                &http_transport_t::on_response<Action>,
                this, action, callback, attempt, std::placeholders::_1
            );
            connection->perform(std::move(action), watcher, settings.timeout);
        } else {
            if (attempt == 1) {
                ES_LOG(log, "no connections - adding default nodes ...");
                add_nodes(settings.endpoints);
                loop.post(
                    std::bind(
                        &http_transport_t::perform<Action>,
                        this, std::move(action), callback, attempt + 1
                    )
                );
            } else {
                ES_LOG(log, "failed: no connections left");
                loop.post(
                    std::bind(
                        callback,
                        result_type(error_t(generic_error_t("no connections left")))
                    )
                );
            }
        }
    }

private:
    template<class Action>
    void on_response(Action action,
                     typename callback<Action>::type callback,
                     int attempt,
                     typename Action::result_type&& result) {
        if (error_t* error = boost::get<error_t>(&result)) {
            switch (error->which()) {
            case 0:
                on_connection_error<Action>(
                    std::move(action),
                    callback,
                    attempt,
                    boost::get<connection_error_t>(*error)
                );
                break;
            case 1:
                on_generic_error(boost::get<generic_error_t>(*error));
                callback(std::move(result));
                break;
            default:
                BOOST_ASSERT(false);
            }
        } else {
            callback(std::move(result));
        }
    }

    template<class Action>
    typename std::enable_if<
        !std::is_same<Action, actions::nodes_info_t>::value
    >::type
    on_connection_error(Action action,
                        typename callback<Action>::type callback,
                        int attempt,
                        const connection_error_t& err) {
        ES_LOG(log, "request failed with error: %s", err.reason);

        remove_node(err.endpoint);
        if (settings.sniffer.when.error) {
            ES_LOG(log, "sniff.on.error is true - preparing to update nodes list");
            sniff(
                std::bind(
                    &http_transport_t::perform<Action>,
                    this, std::move(action), callback, attempt + 1
                )
            );
        } else {
            perform(std::move(action), callback, attempt + 1);
        }
    }

    template<class Action>
    typename std::enable_if<
        std::is_same<Action, actions::nodes_info_t>::value
    >::type
    on_connection_error(Action,
                        typename callback<Action>::type callback,
                        int,
                        const connection_error_t& err) {
        ES_LOG(log, "request failed with error: %s", err.reason);

        remove_node(err.endpoint);
        callback(error_t(err));
    }

    void on_generic_error(const generic_error_t& err) {
        ES_LOG(log, "request failed with error: %s", err.reason);
    }

    void on_sniff(result_t<response::nodes_info_t>::type result,
                  std::function<void()> next) {
        if (auto* info = boost::get<response::nodes_info_t>(&result)) {
            ES_LOG(log, "successfully sniffed %d node%s",
                info->nodes.size(),
                info->nodes.size() > 1 ? "s" : ""
            );

            std::set<std::string> addresses;
            const auto& nodes = info->nodes;
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                const response::node_t& node = it->second;
                extract_addresses(addresses, node.addresses);
            }

            std::vector<endpoint_type> endpoints;
            for (auto it = addresses.begin(); it != addresses.end(); ++it) {
                try {
                    endpoints.push_back(
                        resolver<
                            typename connection_type::protocol_type
                        >::resolve(*it, loop)
                    );
                } catch (const std::exception& err) {
                    ES_LOG(log, "failed to resolve %s address: %s", *it, err.what());
                }
            }

            add_nodes(endpoints);
        }

        next();
    }

    void on_sniff_timer(const boost::system::error_code& ec) {
        if (ec) {
            ES_LOG(log, "sniff on timer event failed: %s", ec.message());
        } else {
            ES_LOG(log, "processing sniff on timer event ...");
            sniff();
        }
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
            ES_LOG(log, "unknown address type: %s", address);
        }
    }
};

} // namespace elasticsearch

