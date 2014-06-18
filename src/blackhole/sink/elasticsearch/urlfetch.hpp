#pragma once

#include <array>

#include <boost/asio.hpp>

#include <urdl/option_set.hpp>
#include <urdl/read_stream.hpp>

#include <blackhole/utils/atomic.hpp>

namespace urlfetch {

struct request_t {
    std::string url;
    urdl::option_set options;
    boost::posix_time::milliseconds timeout;

    request_t() : timeout(1000) {}
};

struct response_t {
    int status;
    std::string data;
};

template<class Stream = urdl::read_stream>
class task_t : public std::enable_shared_from_this<task_t<Stream>> {
public:
    typedef Stream stream_type;
    typedef boost::asio::io_service loop_type;
    typedef boost::posix_time::milliseconds timeout_type;
    typedef std::function<
        void(request_t&&, response_t&&, const boost::system::error_code&)
    > callback_type;

private:
    char buffer[4096];

    request_t request;
    response_t response;
    callback_type callback;

    stream_type stream_;
    boost::asio::deadline_timer timer;

public:
    task_t(request_t request, callback_type callback, loop_type& loop) :
        request(std::move(request)),
        callback(std::move(callback)),
        stream_(loop),
        timer(loop)
    {
        response.data.reserve(16384);
    }

    stream_type& stream() {
        return stream_;
    }

    void run() {
        if (stream_.is_open()) {
            return;
        }

        stream_.set_options(request.options);
        stream_.async_open(
            request.url,
            std::bind(
                &task_t::on_open,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );

        timer.expires_from_now(request.timeout);
        timer.async_wait(
            std::bind(
                &task_t::on_timeout,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void cancel() {
        stream_.close();
    }

private:
    void on_open(const boost::system::error_code& ec) {
        if (ec.category() == urdl::http::error_category() && ec.value() >= 100) {
            response.status = ec.value();
        }

        if (ec && ec.category() != urdl::http::error_category()) {
            timer.cancel();
            callback(std::move(request), std::move(response), ec);
            return;
        }

        async_read_some();
    }

    void on_read(const boost::system::error_code& ec, std::size_t length) {
        response.data.append(buffer, length);
        if (ec) {
            timer.cancel();
            callback(
                std::move(request),
                std::move(response),
                make_error_code(ec)
            );
            return;
        }

        async_read_some();
    }

    void on_timeout(const boost::system::error_code& ec) {
        if (!ec) {
            stream_.close();
        }
    }

    boost::system::error_code
    make_error_code(const boost::system::error_code& ec) const {
        switch (ec.value()) {
        case boost::asio::error::eof:
            return boost::system::error_code();
        case boost::asio::error::operation_aborted:
            if (timer.expires_from_now() > boost::posix_time::time_duration()) {
                return ec;
            } else {
                return boost::asio::error::make_error_code(
                    boost::asio::error::timed_out
                );
            }
        default:
            return ec;
        }

        return ec;
    }

    void async_read_some() {
        stream_.async_read_some(
            boost::asio::buffer(buffer),
            std::bind(
                &task_t::on_read,
                this->shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        );
    }
};

inline
std::shared_ptr<task_t<>>
execute(request_t request,
        task_t<>::callback_type callback,
        task_t<>::loop_type& loop) {
    auto task = std::make_shared<task_t<>>(std::move(request), callback, loop);
    task->run();
    return task;
}

inline
std::shared_ptr<task_t<>>
get(std::string url,
    task_t<>::callback_type callback,
    task_t<>::loop_type& loop,
    task_t<>::timeout_type timeout = task_t<>::timeout_type(1000)) {
    request_t request;
    request.url = url;
    request.timeout = timeout;
    return execute(std::move(request), callback, loop);
}

inline
std::shared_ptr<task_t<>>
get(std::string url,
    task_t<>::callback_type callback,
    task_t<>::loop_type& loop,
    long timeout = 1000) {
    return get(std::move(url), callback, loop, task_t<>::timeout_type(timeout));
}

inline
std::shared_ptr<task_t<>>
put(std::string url,
    std::string body,
    task_t<>::callback_type callback,
    task_t<>::loop_type& loop,
    task_t<>::timeout_type timeout = task_t<>::timeout_type(1000)) {

    request_t request;
    request.url = std::move(url);
    request.timeout = timeout;
    request.options.set_option(urdl::http::request_method("PUT"));
    request.options.set_option(urdl::http::request_content(std::move(body)));
    return execute(std::move(request), callback, loop);
}

inline
std::shared_ptr<task_t<>>
put(std::string url,
    std::string body,
    task_t<>::callback_type callback,
    task_t<>::loop_type& loop,
    long timeout = 1000) {
    return put(
        std::move(url),
        std::move(body),
        callback,
        loop,
        task_t<>::timeout_type(timeout)
    );
}

inline
std::shared_ptr<task_t<>>
post(std::string url,
     std::string body,
     task_t<>::callback_type callback,
     task_t<>::loop_type& loop,
     task_t<>::timeout_type timeout = task_t<>::timeout_type(1000)) {
    request_t request;
    request.url = url;
    request.timeout = timeout;
    request.options.set_option(urdl::http::request_method("POST"));
    request.options.set_option(urdl::http::request_content(std::move(body)));
    return execute(std::move(request), callback, loop);
}

inline
std::shared_ptr<task_t<>>
post(std::string url,
     std::string body,
     task_t<>::callback_type callback,
     task_t<>::loop_type& loop,
     long timeout = 1000) {
    return post(
        std::move(url),
        std::move(body),
        callback,
        loop, task_t<>::timeout_type(timeout)
    );
}

} // namespace urlfetch
