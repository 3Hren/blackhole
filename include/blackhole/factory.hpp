#pragma once

#include <memory>

#include "forward.hpp"

namespace blackhole {
inline namespace v1 {

class factory_t {
public:
    factory_t() = default;
    factory_t(const factory_t& other) = default;
    factory_t(factory_t&& other) = default;

    virtual ~factory_t() = default;

    virtual auto type() const noexcept -> const char* = 0;
};

template<typename T>
class factory : public factory_t {
public:
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<T> = 0;
};

}  // namespace v1
}  // namespace blackhole
