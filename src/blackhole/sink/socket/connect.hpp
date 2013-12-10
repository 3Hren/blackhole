#pragma once

#include <cstdint>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "blackhole/error.hpp"

namespace blackhole {

namespace sink {

//! Resolves specified host and tries to connect to the socket.
template<typename Protocol>
void
connect(boost::asio::io_service& io_service, typename Protocol::socket& socket, const std::string& host, std::uint16_t port) {
    try {
        typename Protocol::resolver resolver(io_service);
        typename Protocol::resolver::query query(host, boost::lexical_cast<std::string>(port));
        typename Protocol::resolver::iterator it = resolver.resolve(query);

        try {
            boost::asio::connect(socket, it);
        } catch (const boost::system::system_error& err) {
            throw error_t("couldn't connect to the %s:%d - %s", host, port, err.what());
        }
    } catch (const boost::system::system_error& err) {
        throw error_t("couldn't resolve %s:%d - %s", host, port, err.what());
    }
}

} // namespace sink

} // namespace blackhole
