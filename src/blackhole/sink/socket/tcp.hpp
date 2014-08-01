#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>

#include "blackhole/utils/unique.hpp"

#include "backend.hpp"
#include "connect.hpp"

namespace blackhole {

namespace sink {

namespace socket {

template<>
class boost_backend_t<boost::asio::ip::tcp> {
public:
    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::socket socket_type;

private:
    const std::string host;
    const std::uint16_t port;

    boost::asio::io_service service;
    std::unique_ptr<socket_type> socket;

public:
    boost_backend_t(std::string host, std::uint16_t port) :
        host(std::move(host)),
        port(port),
        socket(initialize(service, this->host, this->port))
    {}

    static const char* name() {
        return "tcp";
    }

    ssize_t write(const std::string& message) {
        if (!socket) {
            socket = initialize(service, host, port);
        }

        try {
            return boost::asio::write(
                *socket,
                boost::asio::buffer(message.data(), message.size())
            );
        } catch (const boost::system::system_error&) {
            socket.release();
            std::rethrow_exception(std::current_exception());
        }
    }

private:
    static inline
    std::unique_ptr<socket_type>
    initialize(boost::asio::io_service& service,
               const std::string& host,
               std::uint16_t port)
    {
        auto socket = utils::make_unique<socket_type>(service);
        connect<protocol_type>(service, *socket, host, port);
        return socket;
    }
};

} // namespace socket

} // namespace sink

} // namespace blackhole
