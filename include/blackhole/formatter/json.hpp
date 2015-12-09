#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

namespace json {

/// Represents a builder to ease JSON formatter configuration.
///
/// Exists mainly for both avoiding hundreds of constructors and keep its semantics immutable.
class config_t {
    class inner_t;
    std::unique_ptr<inner_t> inner;

public:
    config_t();
    config_t(const config_t& other) = delete;
    config_t(config_t&& other);

    ~config_t();

    auto operator=(const config_t& other) -> config_t& = delete;
    auto operator=(config_t&& other) -> config_t&;

    auto route(std::string rest) -> config_t&;
    auto route(std::string route, std::vector<std::string> attributes) -> config_t&;
    auto rename(std::string from, std::string to) -> config_t&;
    auto unique() -> config_t&;
    auto newline() -> config_t&;

    auto config() const noexcept -> const inner_t&;
};

}  // namespace json

// TODO: Add severity mapping support.
// TODO: Add timestamp mapping support.
// TODO: Take a doc from site.
class json_t : public formatter_t {
public:
    typedef json::config_t config_type;

private:
    template<typename>
    class builder;

    class factory_t;
    std::unique_ptr<factory_t> factory;

public:
    json_t();
    json_t(config_type config);

    ~json_t();

    auto format(const record_t& record, writer_t& writer) -> void;
};

}  // namespace formatter
}  // namespace blackhole
