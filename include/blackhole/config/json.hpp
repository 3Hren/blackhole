#pragma once

#include "factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace config {

class json_t;

template<>
class factory_traits<json_t> {
public:
    /// Constructs and initializes the JSON config factory by reading the given stream reference
    /// until EOF and parsing its content.
    ///
    /// The content should be valid JSON object.
    ///
    /// \param stream lvalue reference to the input stream.
    static auto construct(std::istream& stream) -> std::unique_ptr<factory_t>;

    /// Constructs and initializes the JSON config factory by reading the given stream  until EOF
    /// and parsing its content.
    ///
    /// The content should be valid JSON object.
    ///
    /// \overload
    /// \param stream rvalue reference to the input stream.
    static auto construct(std::istream&& stream) -> std::unique_ptr<factory_t>;
};

}  // namespace config
}  // namespace v1
}  // namespace blackhole
