#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>

#include "blackhole/sink/socket/connect.hpp"
#include "blackhole/utils/unique.hpp"

namespace blackhole {

namespace sink {

namespace socket {

template<>
class boost_backend_t<boost::asio::ip::tcp> {
    typedef boost::asio::ip::tcp Protocol;

    const std::string host;
    const std::uint16_t port;

    boost::asio::io_service io_service;
    std::unique_ptr<Protocol::socket> socket;

public:
    boost_backend_t(const std::string& host, std::uint16_t port) :
        host(host),
        port(port),
        socket(initialize(io_service, host, port))
    {
    }

    ssize_t write(const std::string& message) {
        if (!socket) {
            socket = initialize(io_service, host, port);
        }

        try {
            return socket->write_some(boost::asio::buffer(message.data(), message.size()));
        } catch (const boost::system::system_error& err) {
            socket.release();
            std::rethrow_exception(std::current_exception());
        }
    }

private:
    static inline
    std::unique_ptr<Protocol::socket>
    initialize(boost::asio::io_service& io_service, const std::string& host, std::uint16_t port) {
        std::unique_ptr<Protocol::socket> socket = std::make_unique<Protocol::socket>(io_service);
        connect<Protocol>(io_service, *socket, host, port);
        return socket;
    }
};

} // namespace socket

} // namespace sink

} // namespace blackhole
