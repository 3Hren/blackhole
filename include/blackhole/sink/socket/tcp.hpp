#pragma once

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

/// The TCP socket sink is a sink that writes its output to a remote destination specified by a host
/// and port.
///
/// \remark All methods of this class are thread-safe.
class tcp_t;

}  // namespace socket
}  // namespace sink

template<>
struct factory<sink::socket::tcp_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::socket::tcp_t;
};

}  // namespace v1
}  // namespace blackhole
