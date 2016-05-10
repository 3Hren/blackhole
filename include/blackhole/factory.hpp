#pragma once

#include <memory>

#include "forward.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {

class factory_t {
public:
    factory_t() = default;
    factory_t(const factory_t& other) = default;
    factory_t(factory_t&& other) = default;

    virtual ~factory_t() {}

    /// Returns factory type as a string literal, that is used to identity one factory of a given
    /// type from another.
    virtual auto type() const noexcept -> const char* = 0;
};

template<typename T>
class factory : public factory_t {
public:
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<T> = 0;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
