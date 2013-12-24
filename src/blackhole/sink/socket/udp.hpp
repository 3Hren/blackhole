#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>

#include "backend.hpp"
#include "blackhole/utils/unique.hpp"
#include "connect.hpp"

namespace blackhole {

namespace sink {

namespace socket {

template<>
class boost_backend_t<boost::asio::ip::udp> {
    typedef boost::asio::ip::udp Protocol;

    const std::string m_host;

    boost::asio::io_service m_io_service;
    std::unique_ptr<Protocol::socket> m_socket;

public:
    boost_backend_t(const std::string& host, std::uint16_t port) :
        m_host(host),
        m_socket(initialize(m_io_service, host, port))
    {
    }

    ssize_t write(const std::string& message) {
        return m_socket->send(boost::asio::buffer(message.data(), message.size()));
    }

private:
    static inline
    std::unique_ptr<Protocol::socket>
    initialize(boost::asio::io_service& io_service, const std::string& host, std::uint16_t port) {
        auto socket = std::make_unique<Protocol::socket>(io_service);
        connect<Protocol>(io_service, *socket.get(), host, port);
        return socket;
    }
};

} // namespace socket

} // namespace sink

} // namespace blackhole
