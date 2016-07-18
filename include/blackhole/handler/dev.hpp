#pragma once

#include "../factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace handler {

class dev_t;

}  // namespace handler

template<>
class builder<handler::dev_t> {
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> d;

public:
    builder();

    auto build() && -> std::unique_ptr<handler_t>;
};

template<>
class factory<handler::dev_t> : public factory<handler_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<handler_t> override;
};

}  // namespace v1
}  // namespace blackhole
