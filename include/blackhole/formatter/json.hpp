#pragma once

#include <memory>
#include <string>
#include <vector>

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

// TODO: Add severity mapping support.
// TODO: Add timestamp mapping support.
// TODO: Take a doc from site.
class json_t : public formatter_t {
    class inner_t;
    class properties_t;

    std::unique_ptr<inner_t> inner;

public:
    /// Represents a JSON formatter object builder to ease its configuration.
    class builder_t;

    /// Constructs a defaultly configured JSON formatter, which will produce plain trees with no
    /// filtering without adding a separator character at the end.
    json_t();

    /// Copy constructing is explicitly prohibited.
    json_t(const json_t& other) = delete;

    /// Constructs a JSON formatter using the given other JSON formatter by moving its content.
    json_t(json_t&& other) noexcept;

    /// Destroys the current JSON formatter instance, freeing all its resources.
    ~json_t();

    /// Copy assignment is explicitly prohibited.
    auto operator=(const json_t& other) -> json_t& = delete;

    /// Assigns the given JSON formatter to the current one by moving its content.
    auto operator=(json_t&& other) noexcept -> json_t&;

    /// Formats the given record by constructing a JSON tree with further serializing into the
    /// specified writer.
    auto format(const record_t& record, writer_t& writer) -> void;

private:
    json_t(properties_t properties);
};

/// Represents a JSON formatter object builder to ease its configuration.
///
/// Exists mainly for both avoiding hundreds of formatter constructors and keep its semantics
/// immutable. It's quite convenient to build up different formatter objects just by chaining the
/// different "setters" – no need for default parameters, dealing with constructor bloat etc.
class json_t::builder_t {
    std::unique_ptr<json_t::properties_t> properties;

public:
    builder_t();
    ~builder_t();

    auto route(std::string route) -> builder_t&;
    auto route(std::string route, std::vector<std::string> attributes) -> builder_t&;
    auto rename(std::string from, std::string to) -> builder_t&;
    auto unique() -> builder_t&;
    auto newline() -> builder_t&;

    auto build() const -> json_t;
};

}  // namespace formatter
}  // namespace blackhole
