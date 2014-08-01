#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "blackhole/error.hpp"

namespace blackhole {

namespace aux {

template<typename Protocol, typename SocketService, typename Iterator>
Iterator connect(boost::asio::basic_socket<Protocol, SocketService>& s,
                 Iterator begin,
                 Iterator end,
                 boost::system::error_code& ec)
{
    ec = boost::system::error_code();

    for (Iterator iter = begin; iter != end; ++iter) {
        if (iter != end) {
            s.close(ec);
            s.connect(*iter, ec);
            if (!ec) {
                return iter;
            }
        }
    }

    if (!ec) {
        ec = boost::asio::error::not_found;
    }

    return end;
}

template <typename Protocol, typename SocketService, typename Iterator>
Iterator connect(boost::asio::basic_socket<Protocol, SocketService>& s,
                 Iterator begin)
{
    boost::system::error_code ec;
    Iterator end = typename Protocol::resolver::iterator();
    Iterator result = aux::connect(s, begin, end, ec);
    boost::asio::detail::throw_error(ec);
    return result;
}

} //namespace aux

namespace sink {

//! Resolves specified host and tries to connect to the socket.
template<typename Protocol>
void
connect(boost::asio::io_service& io_service,
        typename Protocol::socket& socket,
        const std::string& host,
        std::uint16_t port)
{
    try {
        typename Protocol::resolver resolver(io_service);
        typename Protocol::resolver::query query(host, boost::lexical_cast<std::string>(port));
        typename Protocol::resolver::iterator it = resolver.resolve(query);

        try {
            aux::connect(socket, it);
        } catch (const boost::system::system_error& err) {
            throw error_t("couldn't connect to the %s:%d - %s", host, port, err.what());
        }
    } catch (const boost::system::system_error& err) {
        throw error_t("couldn't resolve %s:%d - %s", host, port, err.what());
    }
}

} // namespace sink

} // namespace blackhole
