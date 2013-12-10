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
    typedef boost::asio::ip::tcp Protocol;

    const std::string m_host;
    const std::uint16_t m_port;

    boost::asio::io_service m_io_service;
    std::unique_ptr<Protocol::socket> m_socket;

public:
    boost_backend_t(const std::string& host, std::uint16_t port) :
        m_host(host),
        m_port(port),
        m_socket(initialize(m_io_service, host, port))
    {
    }

    ssize_t write(const std::string& message) {
        if (!m_socket) {
            m_socket = initialize(m_io_service, m_host, m_port);
        }

        try {
            return m_socket->write_some(boost::asio::buffer(message.data(), message.size()));
        } catch (const boost::system::system_error&) {
            m_socket.release();
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
