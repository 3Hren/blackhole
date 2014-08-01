#pragma once

#include "socket/udp.hpp"
#include "socket/tcp.hpp"

#include "blackhole/repository/factory/traits.hpp"
#include "blackhole/sink/thread.hpp"

namespace blackhole {

namespace sink {

namespace socket {

struct config_t {
    std::string host;
    std::uint16_t port;
};

} // namespace socket

template<typename Protocol, typename Backend = socket::boost_backend_t<Protocol>>
class socket_t {
public:
    typedef Protocol protocol_type;
    typedef Backend backend_type;

    typedef socket::config_t config_type;

private:
    backend_type backend_;

public:
    socket_t(const config_type& config) :
        backend_(config.host, config.port)
    {}

    socket_t(std::string host, std::uint16_t port) :
        backend_(std::move(host), port)
    {}

    static const char* name() {
        return backend_type::name();
    }

    void consume(const std::string& message) {
        backend_.write(message);
    }

#ifdef BLACKHOLE_TESTING
    backend_type& backend() {
        return backend_;
    }
#endif
};

template<>
struct thread_safety<socket_t<boost::asio::ip::udp>> :
    public std::integral_constant<
        thread::safety_t,
        thread::safety_t::safe
    >::type
{};

} // namespace sink

template<typename Protocol>
struct factory_traits<sink::socket_t<Protocol>> {
    typedef sink::socket_t<Protocol> sink_type;
    typedef typename sink_type::config_type config_type;

    static void map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        ex["host"].to(config.host);
        ex["port"].to(config.port);
    }
};

} // namespace blackhole
