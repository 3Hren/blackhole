#pragma once

#include <memory>

#include <boost/assert.hpp>

#include "blackhole/config.hpp"

namespace blackhole {
namespace config {

template<typename>
class monadic;

template<>
class monadic<config_t> {
    std::unique_ptr<config_t> inner;

public:
    monadic();

    explicit monadic(std::unique_ptr<config_t> inner) :
        inner(std::move(inner))
    {
        BOOST_ASSERT(this->inner);
    }

    auto operator[](const std::size_t& idx) const -> monadic<config_t> {
        return inner->operator[](idx);
    }

    auto operator[](const std::string& key) const -> monadic<config_t> {
        return inner->operator[](key);
    }

    auto operator->() -> config_t* {
        return inner.get();
    }

    auto operator*() const -> const config_t& {
        return *inner;
    }
};

template<typename T, typename... Args>
auto make_monadic(Args&&... args) -> monadic<config_t> {
    return monadic<config_t>(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}  // namespace config
}  // namespace blackhole
