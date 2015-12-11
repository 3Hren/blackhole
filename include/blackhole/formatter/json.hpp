#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
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
    class builder_t;

    json_t();
    json_t(const json_t& other) = delete;
    json_t(json_t&& other);

    ~json_t();

    auto format(const record_t& record, writer_t& writer) -> void;

private:
    json_t(properties_t properties);
};

/// Represents a builder to ease JSON formatter configuration.
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
