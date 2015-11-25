#pragma once

#include <memory>
#include <string>

#include "blackhole/config/factory.hpp"

/// Forwards.
namespace blackhole {
namespace detail {
namespace config {

class json_t;

}  // namespace config
}  // namespace detail
}  // namespace blackhole

namespace blackhole {
namespace config {

using detail::config::json_t;

/// Represents a JSON based logger configuration factory.
template<>
class factory<json_t> : public factory_t {
    class inner_t;
    std::unique_ptr<inner_t> inner;

public:
    /// Initializes the JSON config factory by reading and parsing a file with the given path.
    ///
    /// The file should contain valid JSON object.
    explicit factory(const std::string& path);

    factory(const factory& other) = delete;
    factory(factory&& other);

    /// Destroys the factory with freeing all its associated resources.
    ~factory();

    /// Returns a const lvalue reference to the root configuration.
    auto config() const -> const config_t&;
};

}  // namespace config
}  // namespace blackhole
