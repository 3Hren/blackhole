#pragma once

#include "../forward.hpp"

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

/// Represents an interface for logger configuration factory.
class factory_t {
public:
    /// Destroys the factory with freeing all its associated resources.
    virtual ~factory_t() = 0;

    /// Returns a const lvalue reference to the root configuration.
    virtual auto config() const -> const node_t& = 0;
};

}  // namespace config
}  // namespace v1
}  // namespace blackhole
