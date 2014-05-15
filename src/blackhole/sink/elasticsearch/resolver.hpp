#pragma once

#include <cstdint>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

namespace elasticsearch {

template<typename Protocol>
class resolver {
public:
    typedef Protocol protocol_type;
    typedef typename protocol_type::endpoint endpoint_type;
    typedef typename protocol_type::resolver resolver_type;
    typedef typename protocol_type::resolver::query query_type;
    typedef typename protocol_type::resolver::iterator iterator_type;

private:
    struct inet_addr_t {
        std::string hostname;
        std::string ip;
        std::uint16_t port;

        inet_addr_t() : port(0) {}
    };

public:
    static
    endpoint_type
    resolve(const std::string& address, boost::asio::io_service& service) {
        inet_addr_t addr = parse(address);
        if (addr.port == 0) {
            throw std::logic_error("port is null");
        }

        if (addr.ip.empty()) {
            if (addr.hostname.empty()) {
                throw std::logic_error("neither hostname nor ip not specified");
            } else {
                return resolve(addr.hostname, addr.port, service);
            }
        }

        return resolve(addr.ip, addr.port, service);
    }

    static
    endpoint_type
    resolve(const std::string& host,
            std::uint16_t port,
            boost::asio::io_service& service) {
        try {
            return resolve(
                host,
                boost::lexical_cast<std::string>(port), service
            );
        } catch (const boost::bad_lexical_cast&) {
            throw std::logic_error("failed to resolve: wrong port value");
        }
    }

    static
    endpoint_type
    resolve(const std::string& host,
            const std::string& port,
            boost::asio::io_service& service) {
        resolver_type resolver(service);
        query_type query(host, port);
        boost::system::error_code ec;
        iterator_type it = resolver.resolve(query, ec);
        iterator_type end;
        if (it == end || ec) {
            throw std::runtime_error("failed to resolve: " + ec.message());
        }

        return *it;
    }

private:
    static inet_addr_t parse(std::string in) {
        inet_addr_t addr;

        std::string::size_type hostname_pos = in.find('/');
        if (hostname_pos != std::string::npos) {
            addr.hostname = std::string(
                in.begin(),
                in.begin() + hostname_pos
            );
        }

        std::string::size_type ip_pos = in.rfind(':');
        if (ip_pos != std::string::npos) {
            addr.ip = std::string(
                in.begin() + hostname_pos + 1,
                in.begin() + ip_pos
            );
        }

        try {
            addr.port = boost::lexical_cast<std::uint16_t>(
                std::string(
                    in.begin() + ip_pos + 1,
                    in.end()
                )
            );
        } catch (const boost::bad_lexical_cast&) {}

        return addr;
    }
};

} // namespace elasticsearch
