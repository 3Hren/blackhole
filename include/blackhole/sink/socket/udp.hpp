#pragma once

#include "blackhole/sink.hpp"

#include <memory>

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

namespace udp {

class inner_t;

}  // namespace udp

class udp_t : public sink_t {
    std::unique_ptr<udp::inner_t> inner;

public:
    udp_t(const std::string& host, std::uint16_t port);

    udp_t(const udp_t& other) = delete;
    udp_t(udp_t&& other) noexcept;

    ~udp_t();

    auto operator=(const udp_t& other) -> udp_t& = delete;
    auto operator=(udp_t&& other) noexcept -> udp_t&;

    auto filter(const record_t& record) -> bool;

    auto execute(const record_t& record, const string_view& formatted) -> void;
};

// builder<socket::udp_t>()
//     .host(std::string)
//     .port(std::uint16_t)
//     .nonblocking()
//     .build();

}  // namespace socket
}  // namespace sink

template<>
struct factory<sink::socket::udp_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::socket::udp_t;
};

}  // namespace v1
}  // namespace blackhole
