#pragma once

#include <boost/asio/ip/udp.hpp>

#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

class udp_t : public sink_t {
public:
    /// The endpoint type.
    typedef boost::asio::ip::basic_endpoint<boost::asio::ip::udp> endpoint_type;

private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint endpoint_;

public:
    udp_t(const std::string& host, std::uint16_t port);

    /// Returns a const lvalue reference to the destination endpoint.
    auto endpoint() const -> const endpoint_type&;

    /// Emits a datagram to the specified endpoint.
    auto emit(const record_t& record, const string_view& message) -> void override;
};

}  // namespace socket
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
