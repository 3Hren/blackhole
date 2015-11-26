#pragma once

#include <memory>

namespace blackhole {

class config_t;

namespace config {

template<typename>
class monadic;

template<>
class monadic<config_t> {
    std::unique_ptr<config_t> inner;

public:
    monadic();
    explicit monadic(std::unique_ptr<config_t> inner);
    monadic(monadic&& other) = default;

    ~monadic();

    /// Checks whether the inner value is initialized.
    explicit operator bool() const;

    auto operator[](const std::size_t& idx) const -> monadic<config_t>;
    auto operator[](const std::string& key) const -> monadic<config_t>;
    auto operator->() -> config_t*;
    auto operator*() const -> const config_t&;
};

template<typename T, typename... Args>
auto make_monadic(Args&&... args) -> monadic<config_t> {
    return monadic<config_t>(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}  // namespace config
}  // namespace blackhole
