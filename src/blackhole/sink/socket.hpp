#pragma once

#include "socket/udp.hpp"
#include "socket/tcp.hpp"

namespace blackhole {

namespace sink {

template<typename Protocol, typename Backend = socket::boost_backend_t<Protocol>>
class socket_t {
    Backend m_backend;

public:
    socket_t(const std::string& host, std::uint16_t port) :
        m_backend(host, port)
    {
    }

    void consume(const std::string& message) {
        m_backend.write(message);
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

} // namespace blackhole
