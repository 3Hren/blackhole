#pragma once

namespace blackhole {
namespace config {

class node_t;

/// Represents an interface for logger configuration factory.
class factory_t {
public:
    factory_t() = default;
    factory_t(const factory_t& other) = default;
    factory_t(factory_t&& other) = default;

    /// Destroys the factory with freeing all its associated resources.
    virtual ~factory_t() = 0;

    auto operator=(const factory_t& other) -> factory_t& = default;
    auto operator=(factory_t&& other) -> factory_t& = default;

    /// Returns a const lvalue reference to the root configuration.
    virtual auto config() const -> const node_t& = 0;
};

/// Concrete factory implementations template.
template<typename>
class factory;

}  // namespace config
}  // namespace blackhole
