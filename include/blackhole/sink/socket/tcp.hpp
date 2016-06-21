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

template<>
class factory<sink::socket::tcp_t> : public factory<sink_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace v1
}  // namespace blackhole
