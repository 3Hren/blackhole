#pragma once

#include "blackhole/factory.hpp"
#include "blackhole/forward.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {

class asynchronous_t;

}  // namespace sink

template<>
class factory<sink::asynchronous_t> : public experimental::factory<sink_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    virtual auto type() const noexcept -> const char* override;
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
