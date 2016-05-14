#pragma once

#include <memory>
#include <string>
#include <vector>

#include "blackhole/formatter.hpp"

namespace blackhole {
inline namespace v1 {

template<typename>
struct factory;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole

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
///     auto formatter = json_t::builder_t()
///         .route("/fields", {"message", "severity", "timestamp"})
///         .route("/other")
///         .rename("message", "#message")
///         .rename("timestamp", "#timestamp")
///         .newline()
///         .unique()
///         .build();
///
/// This allow to avoid hundreds of constructors and to make a formatter creation to look eye-candy.
// TODO: Idea. Make class private. Export only forward with factory and builder specializations. It
//      saves from that hundreds of PIMPLS.
// builder<T>::build() -> std::unique_ptr<formatter_t>;
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

    /// Returns true if there will be newline sequence added after each formatted message.
    auto newline() const noexcept -> bool;

    /// Returns true if the filtering policy is enabled.
    auto unique() const noexcept -> bool;

    /// Saves the given severity mapping array.
    ///
    /// Each formatting iteration will trigger a severity mutation by changing its integral
    /// representation with the corresponding string.
    ///
    /// \param sevmap severity mapping array.
    /// \note setting an empty array resets the behavior.
    /// \remark this method is not thread safe; setting a severity mapping while formatting will
    ///     result in undefined behavior.
    auto severity(std::vector<std::string> sevmap) -> void;

    auto timestamp(const std::string& pattern) -> void;

    /// Formats the given record by constructing a JSON tree with further serializing into the
    /// specified writer.
    ///
    /// \remark this method is thread safe.
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

    /// Configures attribute routing for all not mentioned attributes.
    auto route(std::string route) -> builder_t&;

    /// Configures attribute routing for the given set of attributes.
    ///
    /// Routing feature allows to build a JSON tree using simple key -> set representation. Each
    /// attribute in the set will be traversed into the specified route.
    /// Route parameter adheres JSON Pointer RFC, see https://tools.ietf.org/html/rfc6901 for more
    /// information.
    auto route(std::string route, std::vector<std::string> attributes) -> builder_t&;
    auto rename(std::string from, std::string to) -> builder_t&;
    auto unique() -> builder_t&;
    auto newline() -> builder_t&;
    // TODO: auto mutate(...) -> builder_t&;

    /// \warning experimental, may be dropped until 1.0.
    auto severity(std::vector<std::string> sevmap) -> builder_t&;

    /// \warning experimental, may be dropped until 1.0.
    auto timestamp(const std::string& pattern) -> builder_t&;

    auto build() const -> json_t;
};

}  // namespace formatter

template<>
struct factory<formatter::json_t> {
    static auto type() noexcept -> const char*;
    static auto from(const config::node_t& config) -> formatter::json_t;
};

}  // namespace v1
}  // namespace blackhole
