#pragma once

#include <memory>
#include <vector>

#include "../factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {

/// The JSON formatter is responsible for effective converting the given log record into a
/// structured JSON tree with attributes routing and renaming features.
///
/// Briefly using JSON formatter allows to build fully dynamic JSON trees for its further processing
/// with various external tools, like logstash or rsyslog lefting it, however, in a human-readable
/// state.
///
/// Blackhole allows you to control of JSON tree building process using several predefined options.
///
/// Without options it will produce just a plain tree with zero level depth.
/// For example for a log record with a severity of 3, message "fatal error, please try again" and a
/// pair of attributes `{"key": 42, "ip": "[::]"}` the result string will look like:
/// {
///     "message": "fatal error, please try again",
///     "severity": 3,
///     "timestamp": 1449859055,
///     "process": 12345,
///     "thread": 0x0000dead,
///     "key": 42,
///     "ip": "[::]"
/// }
///
/// Using configuration parameters for this formatter you can:
/// - Rename parameters.
/// - Construct hierarchical tree using a standardized JSON pointer API. For more information please
///   follow \ref https://tools.ietf.org/html/rfc6901.
///
/// Attributes renaming acts so much transparently as it appears: it just renames the given
/// attribute name using the specified alternative.
///
/// Attributes routing specifies a location where the listed attributes will be placed at the tree
/// construction. Also you can specify a default location for all attributes, which is "/" meaning
/// root otherwise.
///
/// For example with routing `{"/fields": ["message", "severity"]}` and "/" as a default pointer the
/// mentioned JSON will look like:
/// {
///     "fields": {
///         "message": "fatal error, please try again",
///         "severity": 3
///     },
///     "timestamp": 1449859055,
///     "process": 12345,
///     "thread": 0x0000dead,
///     "key": 42,
///     "ip": "[::]"
/// }
///
/// Attribute renaming occurs after routing, so mapping "message" => "#message" just replaces the
/// old name with its new alternative.
///
/// To gain maximum speed at the tree construction no filtering occurs, so this formatter by default
/// allows duplicated keys, which means invalid JSON tree (but most of parsers are fine with it).
/// If you are really required to deal with unique keys, you can enable `unique` option, but it
/// involves heap allocation and may slow down formatting.
///
/// Also formatter allows to automatically append a newline character at the end of the tree, which
/// is strangely required by some consumers, like logstash.
///
/// Note, that JSON formatter formats the tree using compact style without excess spaces, tabs etc.
///
/// For convenient formatter construction a special builder class is implemented allowing to create
/// and configure instances of this class using streaming API. For example:
///     auto formatter = builder<json_t>()
///         .route("/fields", {"message", "severity", "timestamp"})
///         .route("/other")
///         .rename("message", "#message")
///         .rename("timestamp", "#timestamp")
///         .newline()
///         .unique()
///         .build();
///
/// This allow to avoid hundreds of constructors and to make a formatter creation to look eye-candy.
class json_t;

}  // namespace formatter

namespace experimental {

/// Represents a JSON formatter object builder to ease its configuration.
///
/// Exists mainly for both avoiding hundreds of formatter constructors and keep its semantics
/// immutable. It's quite convenient to build up different formatter objects just by chaining the
/// different "setters" – no need for default parameters, dealing with constructor bloat etc.
template<>
class builder<formatter::json_t> {
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> d;

public:
    builder();

    /// Configures attribute routing for all not mentioned attributes.
    auto route(std::string route) & -> builder&;
    auto route(std::string route) && -> builder&&;

    /// Configures attribute routing for the given set of attributes.
    ///
    /// Routing feature allows to build a JSON tree using simple key -> set representation. Each
    /// attribute in the set will be traversed into the specified route.
    /// Route parameter adheres JSON Pointer RFC, see https://tools.ietf.org/html/rfc6901 for more
    /// information.
    auto route(std::string route, std::vector<std::string> attributes) & -> builder&;
    auto route(std::string route, std::vector<std::string> attributes) && -> builder&&;

    auto rename(std::string from, std::string to) & -> builder&;
    auto rename(std::string from, std::string to) && -> builder&&;

    auto unique() & -> builder&;
    auto unique() && -> builder&&;

    auto newline() & -> builder&;
    auto newline() && -> builder&&;
    // TODO: auto mutate(...) -> builder&;

    /// Sets severity mapping array.
    ///
    /// Each formatting iteration will trigger a severity mutation by changing its internal
    /// representation with the corresponding string.
    ///
    /// \note setting an empty array resets the behavior.
    /// \warning experimental, may be dropped until 1.0.
    ///
    /// \param sevmap severity mapping array.
    auto severity(std::vector<std::string> sevmap) & -> builder&;
    auto severity(std::vector<std::string> sevmap) && -> builder&&;

    /// \warning experimental, may be dropped until 1.0.
    auto timestamp(const std::string& pattern) & -> builder&;
    auto timestamp(const std::string& pattern) && -> builder&&;

    auto build() const && -> std::unique_ptr<formatter_t>;
};

template<>
class factory<formatter::json_t> : public factory<formatter_t> {
public:
    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<formatter_t> override;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
