#pragma once

#include <functional>

#include <boost/asio.hpp>

#include <rapidjson/document.h>

#include <swarm/logger.h>
#include <swarm/urlfetcher/boost_event_loop.hpp>
#include <swarm/urlfetcher/stream.hpp>
#include <swarm/urlfetcher/url_fetcher.hpp>

#include "log.hpp"
#include "response/extract.hpp"
#include "result.hpp"
#include "request/method.hpp"

namespace elasticsearch {

class http_connection_t;

struct urlfetcher_t {
    typedef boost::asio::io_service loop_type;

    typedef ioremap::swarm::url url_type;
    typedef ioremap::swarm::simple_stream stream_type;
    typedef ioremap::swarm::url_fetcher::request request_type;
    typedef ioremap::swarm::url_fetcher::response response_type;

    loop_type& loop;
    loop_type::work work;
    ioremap::swarm::logger logger;
    ioremap::swarm::boost_event_loop loop_wrapper;
    ioremap::swarm::url_fetcher manager;

    urlfetcher_t(loop_type& loop) :
        loop(loop),
        work(loop),
        loop_wrapper(loop),
        manager(loop_wrapper, logger)
    {}
};

template<class Action>
struct callback {
    typedef std::function<void(typename Action::result_type)> type;
};

template<class Action>
class response_handler_t {
public:
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef Action action_type;
    typedef typename action_type::result_type result_type;
    typedef typename action_type::response_type response_type;
    typedef typename callback<action_type>::type callback_type;

    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::endpoint endpoint_type;

private:
    callback_type callback;
    endpoint_type endpoint;
    logger_type& log;

public:
    response_handler_t(callback_type callback,
                       endpoint_type endpoint,
                       logger_type& log) :
        callback(callback),
        endpoint(endpoint),
        log(log)
    {}

    void operator()(const urlfetcher_t::response_type& response,
                    const std::string& data,
                    const boost::system::error_code& ec) {
        const int status = response.code();

        LOG(log, "response received from '%s' [%d] (%s): %s",
            response.url().to_string(), status, ec.value(), data);

        if (ec) {
            callback(result_type(
                error_t(connection_error_t(endpoint, ec.message()))));
            return;
        }

        rapidjson::Document doc;
        doc.Parse<0>(data.c_str());
        if (doc.HasParseError()) {
            callback(result_type(
                error_t(generic_error_t(doc.GetParseError()))));
            return;
        }

        if (has_error(status)) {
            const rapidjson::Value& error = doc["error"];
            callback(result_type(
                error_t(generic_error_t(
                    error.IsString() ? error.GetString() : "unknown"))));
            return;
        }

        try {
            callback(result_type(
                extractor_t<response_type>::extract(doc)
            ));
        } catch (const std::exception& err) {
            callback(result_type(error_t(generic_error_t(err.what()))));
        }
    }

private:
    static inline bool has_error(int status) {
        const int type = status / 100;
        return type == 4 || type == 5;
    }
};

class http_connection_t {
public:
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef boost::asio::io_service loop_type;
    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::endpoint endpoint_type;

private:
    endpoint_type endpoint_;
    urlfetcher_t& urlfetcher;
    logger_type& log;

public:
    http_connection_t(endpoint_type endpoint,
                      urlfetcher_t& urlfetcher,
                      logger_type& log) :
        endpoint_(endpoint),
        urlfetcher(urlfetcher),
        log(log)
    {
        //!@todo: Make configurable.
        urlfetcher.manager.set_total_limit(20);
    }

    ~http_connection_t() {
    }

    endpoint_type endpoint() const {
        return endpoint_;
    }

    std::string address() const {
        if (endpoint_.address().is_v6()) {
            std::string result;
            result.push_back('[');
            result.append(endpoint().address().to_string());
            result.push_back(']');
            return result;
        }

        return endpoint_.address().to_string();
    }

    std::uint16_t port() const {
        return endpoint_.port();
    }

    template<class Action>
    void perform(Action&& action, typename callback<Action>::type callback) {
        urlfetcher_t::url_type url;
        url.set_scheme("http");
        url.set_host(address());
        url.set_port(port());
        url.set_path(action.path());

        urlfetcher_t::request_type request;
        request.set_url(url);
        request.headers().set_keep_alive();

        response_handler_t<Action> handler(callback, endpoint_, log);
        std::shared_ptr<urlfetcher_t::stream_type> stream = std::move(
            urlfetcher_t::stream_type::create(handler)
        );
        LOG(log, "requesting '%s' ...", request.url().to_string());
        perform(std::move(action), std::move(request), std::move(stream));
    }

private:
    template<class Action>
    typename std::enable_if<
        Action::method_value == request::method_t::get
    >::type
    perform(Action&&,
            urlfetcher_t::request_type&& request,
            std::shared_ptr<urlfetcher_t::stream_type>&& stream) {
        urlfetcher.manager.get(stream, std::move(request));
    }

    template<class Action>
    typename std::enable_if<
        Action::method_value == request::method_t::post
    >::type
    perform(Action&& action,
            urlfetcher_t::request_type&& request,
            std::shared_ptr<urlfetcher_t::stream_type>&& stream) {
        urlfetcher.manager.post(stream, std::move(request), action.body());
    }
};

} // namespace elasticsearch
