#pragma once

#include "../factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace handler {

class blocking_t;

}  // namespace handler

template<>
class builder<handler::blocking_t> {
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> d;

public:
    builder();

    auto set(std::unique_ptr<formatter_t> formatter) & -> builder&;
    auto set(std::unique_ptr<formatter_t> formatter) && -> builder&&;
    auto add(std::unique_ptr<sink_t> sink) & -> builder&;
    auto add(std::unique_ptr<sink_t> sink) && -> builder&&;

    auto build() && -> std::unique_ptr<handler_t>;
};

template<>
class factory<handler::blocking_t> : public factory<handler_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    virtual auto type() const noexcept -> const char* override;
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<handler_t> override;
};

}  // namespace v1
}  // namespace blackhole
