#pragma once

#include <memory>
#include <string>

#include "blackhole/config/factory.hpp"

/// Forwards.
namespace blackhole {
inline namespace v1 {
namespace detail {
namespace config {

class json_t;

}  // namespace config
}  // namespace detail
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

using detail::config::json_t;

/// Represents a JSON based logger configuration factory.
// TODO: Use make_json_factory(...) instead, interfaces are great! Then drop copy/move defaults.
template<>
class factory<json_t> : public factory_t {
    class inner_t;
    std::unique_ptr<inner_t> inner;

public:
    /// Constructs and initializes the JSON config factory by reading the given stream reference
    /// until EOF and parsing its content.
    ///
    /// The content should be valid JSON object.
    ///
    /// \param stream lvalue reference to the input stream.
    explicit factory(std::istream& stream);

    /// Constructs and initializes the JSON config factory by reading the given stream  until EOF
    /// and parsing its content.
    ///
    /// The content should be valid JSON object.
    ///
    /// \overload
    /// \param stream rvalue reference to the input stream.
    explicit factory(std::istream&& stream);

    /// Copy construction is explicitly prohibited.
    factory(const factory& other) = delete;

    /// Constructs the JSON config factory by consuming other JSON factory object.
    factory(factory&& other) noexcept;

    /// Destroys the factory with freeing all its associated resources.
    ~factory();

    /// Copy assignment is explicitly prohibited.
    auto operator=(const factory& other) -> factory& = delete;

    /// Assigns the given JSON factory to the current one by consuming its object.
    auto operator=(factory&& other) noexcept -> factory&;

    /// Returns a const lvalue reference to the root configuration.
    auto config() const noexcept -> const node_t&;

private:
    auto initialize(std::istream& stream) -> void;
};

}  // namespace config
}  // namespace v1
}  // namespace blackhole
