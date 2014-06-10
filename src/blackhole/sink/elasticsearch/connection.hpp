#pragma once

#include <functional>
#include <memory>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include <rapidjson/document.h>

#include <urdl/read_stream.hpp>

#include "log.hpp"
#include "response/extract.hpp"
#include "result.hpp"
#include "request/method.hpp"
#include "urlfetch.hpp"

namespace elasticsearch {

template<class Action>
struct callback {
    typedef std::function<void(typename Action::result_type)> type;
};

template<class Action, class Connection>
class response_handler_t {
public:
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef Action action_type;
    typedef Connection connection_type;

    typedef typename action_type::result_type result_type;
    typedef typename action_type::response_type response_type;
    typedef typename callback<action_type>::type callback_type;
    typedef typename connection_type::endpoint_type endpoint_type;

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

    void
    operator()(urlfetch::request_t&& request,
               urlfetch::response_t&& response,
               const boost::system::error_code& ec) {
        ES_LOG(log, "response received from '%s' [%s]: %s",
               request.url, ec.value(), response.data
        );

        if (ec) {
            callback(
                result_type(
                    error_t(connection_error_t(endpoint, ec.message()))
                )
            );
            return;
        }

        rapidjson::Document doc;
        doc.Parse<0>(response.data.c_str());
        if (doc.HasParseError()) {
            callback(result_type(error_t(generic_error_t(doc.GetParseError()))));
            return;
        }

        const rapidjson::Value& error = doc["error"];
        if (!error.IsNull()) {
            callback(result_type(
                error_t(generic_error_t(
                    error.IsString() ? error.GetString() : "unknown"))
                )
            );
            return;
        }

        try {
            callback(result_type(extractor_t<response_type>::extract(doc)));
        } catch (const std::exception& err) {
            callback(result_type(error_t(generic_error_t(err.what()))));
        }
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
    loop_type& loop;
    logger_type& log;

public:
    http_connection_t(endpoint_type endpoint,
                      loop_type& loop,
                      logger_type& log) :
        endpoint_(endpoint),
        loop(loop),
        log(log)
    {}

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
    void perform(Action&& action,
                 typename callback<Action>::type callback,
                 long timeout) {
        const std::string url = make_url(action.path());
        response_handler_t<
            Action,
            http_connection_t
        > handler(callback, endpoint_, log);
        ES_LOG(log, "requesting '%s' ...", url);
        perform(std::move(action), std::move(handler), url, timeout);
    }

private:
    std::string
    make_url(const std::string& path) const {
        std::string url("http://");
        url.reserve(128);
        url.append(address());
        url.push_back(':');
        url.append(boost::lexical_cast<std::string>(port()));
        url.append(path);
        return url;
    }

    template<class Action>
    typename std::enable_if<
        Action::method::value == request::method_t::get
    >::type
    perform(Action&&,
            response_handler_t<Action, http_connection_t>&& handler,
            std::string url,
            long timeout) {
        urlfetch::get(url, handler, loop, timeout);
    }

    template<class Action>
    typename std::enable_if<
        Action::method::value == request::method_t::post
    >::type
    perform(Action&& action,
            response_handler_t<Action, http_connection_t>&& handler,
            std::string url,
            long timeout) {
        urlfetch::post(url, std::move(action.body()), handler, loop, timeout);
    }
};

} // namespace elasticsearch
