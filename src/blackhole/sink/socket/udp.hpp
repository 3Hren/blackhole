#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>

#include "backend.hpp"
#include "connect.hpp"

namespace blackhole {

namespace sink {

namespace socket {

template<>
class boost_backend_t<boost::asio::ip::udp> {
    typedef boost::asio::ip::udp Protocol;

    const std::string m_host;
    const std::uint16_t m_port;

    boost::asio::io_service m_io_service;
    Protocol::socket m_socket;

public:
    boost_backend_t(const std::string& host, std::uint16_t port) :
        m_host(host),
        m_port(port),
        m_socket(initialize(m_io_service, host, port))
    {
    }

    ssize_t write(const std::string& message) {
        return m_socket.send(boost::asio::buffer(message.data(), message.size()));
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
