#pragma once

#include <mutex>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/socket/tcp.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

namespace detail {

/// Resolves specified host and tries to connect to the socket.
template<typename Protocol>
auto connect(boost::asio::io_service& ev, typename Protocol::socket& socket,
    const std::string& host, std::uint16_t port) -> void
{
    typename Protocol::resolver::iterator endpoint;

    try {
        typename Protocol::resolver resolver(ev);
        typename Protocol::resolver::query query(host, boost::lexical_cast<std::string>(port),
            Protocol::resolver::query::flags::numeric_service);
        endpoint = resolver.resolve(query);
    } catch (const boost::system::system_error& err) {
        throw std::system_error(err.code().value(), std::system_category(),
            fmt::format("failed to resolve {}:{} - {}", host, port, err.what()));
    }

    try {
        boost::asio::connect(socket, endpoint);
    } catch (const boost::system::system_error& err) {
        throw std::system_error(err.code().value(), std::system_category(),
            fmt::format("failed to connect to {}:{} - {}", host, port, err.what()));
    }
}

}  // namespace

class tcp_t : public sink_t {
    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::socket socket_type;
    typedef protocol_type::endpoint endpoint_type;

    struct data_t {
        std::string host;
        std::uint16_t port;

        boost::asio::io_service io_service;
        std::unique_ptr<socket_type> socket;

        mutable std::mutex mutex;
    };

    std::unique_ptr<data_t> data;

public:
    tcp_t(std::string host, std::uint16_t port) :
        data(new data_t)
    {
        data->host = std::move(host);
        data->port = port;
    }

    auto host() const noexcept -> const std::string& {
        return data->host;
    }

    auto port() const noexcept -> std::uint16_t {
        return data->port;
    }

    auto emit(const record_t&, const string_view& message) -> void {
        std::lock_guard<std::mutex> lock(data->mutex);

        if (!data->socket) {
            data->socket = reconnect(lock);
        }

        try {
            boost::asio::write(*data->socket, boost::asio::buffer(message.data(), message.size()));
        } catch (const boost::system::system_error&) {
            data->socket.reset();
            std::rethrow_exception(std::current_exception());
        }
    }

private:
    auto reconnect(std::lock_guard<std::mutex>&) -> std::unique_ptr<socket_type> {
        auto socket = std::unique_ptr<socket_type>(new socket_type(data->io_service));
        detail::connect<protocol_type>(data->io_service, *socket, host(), port());

        return socket;
    }
};

}  // namespace socket
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
