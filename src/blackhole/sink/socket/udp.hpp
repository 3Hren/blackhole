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
public:
    typedef boost::asio::ip::udp protocol_type;
    typedef protocol_type::socket socket_type;

private:
    const std::string host;

    boost::asio::io_service service;
    std::unique_ptr<socket_type> socket;

public:
    boost_backend_t(std::string host, std::uint16_t port) :
        host(std::move(host)),
        socket(initialize(service, this->host, port))
    {
    }

    static const char* name() {
        return "udp";
    }

    ssize_t write(const std::string& message) {
        return socket->send(boost::asio::buffer(message.data(), message.size()));
    }

private:
    static inline
    std::unique_ptr<protocol_type::socket>
    initialize(boost::asio::io_service& service,
               const std::string& host,
               std::uint16_t port)
    {
        auto socket = utils::make_unique<protocol_type::socket>(service);
        connect<protocol_type>(service, *socket, host, port);
        return socket;
    }
};

} // namespace socket

} // namespace sink

} // namespace blackhole
