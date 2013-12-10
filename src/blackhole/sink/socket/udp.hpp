#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>

#include "blackhole/sink/socket/connect.hpp"

namespace blackhole {

namespace sink {

namespace socket {

template<typename Protocol>
class boost_backend_t;

template<>
class boost_backend_t<boost::asio::ip::udp> {
    typedef boost::asio::ip::udp Protocol;

    const std::string host;
    const std::uint16_t port;

    boost::asio::io_service io_service;
    Protocol::socket socket;

public:
    boost_backend_t(const std::string& host, std::uint16_t port) :
        host(host),
        port(port),
        socket(initialize(io_service, host, port))
    {
    }

    ssize_t write(const std::string& message) {
        return socket.send(boost::asio::buffer(message.data(), message.size()));
    }

private:
    static inline
    Protocol::socket
    initialize(boost::asio::io_service& io_service, const std::string& host, std::uint16_t port) {
        Protocol::socket socket(io_service);
        connect<Protocol>(io_service, socket, host, port);
        return socket;
    }
};

} // namespace socket

} // namespace sink

} // namespace blackhole
