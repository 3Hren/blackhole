#pragma once

#include <mutex>

#include <boost/asio/ip/tcp.hpp>

#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

class tcp_t : public sink_t {
    std::string host_;
    std::uint16_t port_;

    boost::asio::io_context io_service;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;

    mutable std::mutex mutex;

public:
    tcp_t(std::string host, std::uint16_t port);

    auto host() const noexcept -> const std::string&;
    auto port() const noexcept -> std::uint16_t;

    auto emit(const record_t& record, const string_view& message) -> void override;
};

}  // namespace socket
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
