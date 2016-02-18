#pragma once

#include <memory>

#include <blackhole/sink.hpp>

namespace blackhole {
inline namespace v1 {

template<typename>
struct factory;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

/// The TCP socket sink is a sink that writes its output to a remote destination specified by a host
/// and port.
///
/// \remark All methods of this class are thread-safe.
class tcp_t : public sink_t {
    struct data_t;
    std::unique_ptr<data_t> data;

public:
    tcp_t(std::string host, std::uint16_t port);

    tcp_t(const tcp_t& other) = delete;
    tcp_t(tcp_t&& other) noexcept;

    ~tcp_t();

    auto operator=(const tcp_t& other) -> tcp_t& = delete;
    auto operator=(tcp_t&& other) noexcept -> tcp_t&;

    auto host() const noexcept -> const std::string&;
    auto port() const noexcept -> std::uint16_t;

    auto emit(const record_t& record, const string_view& message) -> void;
};

}  // namespace socket
}  // namespace sink

template<>
struct factory<sink::socket::tcp_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::socket::tcp_t;
};

}  // namespace v1
}  // namespace blackhole
