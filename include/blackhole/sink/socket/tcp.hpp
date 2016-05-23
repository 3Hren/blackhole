#pragma once

#include "../../factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

/// The TCP socket sink is a sink that writes its output to a remote destination specified by a host
/// and port.
class tcp_t;

}  // namespace socket
}  // namespace sink

namespace experimental {

template<>
struct factory<sink::socket::tcp_t> : public factory<sink_t> {
    auto type() const noexcept -> const char* final override;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> final override;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
