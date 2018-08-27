#pragma once

#include <functional>

#include "../factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {

/// Severity mapping function.
///
/// Default value just writes an integer representation.
///
/// \param severity an integer representation of current log severity.
/// \param spec the format specification as it was provided with the initial pattern.
/// \param writer result writer.
typedef std::function<void(int severity, const std::string& spec, writer_t& writer)> severity_map;

/// The string formatter is responsible for effective converting the given record to a string using
/// precompiled pattern and options.
///
/// This formatter allows to specify the pattern using python-like syntax with braces and attribute
/// names.
///
/// For example, the given pattern `{severity:d}, [{timestamp}]: {message}` would result in
/// something like this: `1, [2015-11-18 15:50:12.630953]: HTTP1.1 - 200 OK`.
///
/// Let's explain what's going on when a log record passed such pattern.
///
/// There are three named arguments or attributes: severity, timestamp and message. The severity
/// is represented as signed integer, because of `:d` format specifier. Other tho arguments haven't
/// such specifiers, so they are represented using default types for each attribute. In our case
/// the timestamp and message attributes are formatted as strings.
///
/// Considering message argument almost everything is clear, but for timestamp there are internal
/// magic comes. There is default `%Y-%m-%d %H:%M:%S.%f` pattern for timestamp attributes which
/// reuses `strftime` standard placeholders with an extension of microseconds - `%f`. It means
/// that the given timestamp is formatted with 4-digit decimal year, 1-12 decimal month and so on.
///
/// See \ref http://en.cppreference.com/w/cpp/chrono/c/strftime for more details.
///
/// The formatter uses python-like syntax with all its features, like aligning, floating point
/// precision etc.
///
/// For example the `{re:+.3f}; {im:+.6f}` pattern is valid and results in `+3.140; -3.140000`
/// message with `re: 3.14` and `im: -3.14` attributes provided.
///
/// For more information see \ref http://cppformat.github.io/latest/syntax.html resource.
///
/// With a few predefined exceptions the formatter supports all userspace attributes. The exceptions
/// are: message, severity, timestamp, process and thread. For these attributes there are special
/// rules and it's impossible to override then even with the same name attribute.
///
/// For message attribute there are no special rules. It's still allowed to extend the specification
/// using fill, align and other specifiers.
///
/// With timestamp attribute there is an extension of either explicitly providing formatting pattern
/// or forcing the attibute to be printed as an integer.
/// In the first case for example the pattern may be declared as `{timestamp:{%Y}s}` which results
/// in only year formattinh using YYYY style.
/// In the second case one can force the timestamp to be printed as microseconds passed since epoch.
///
/// Severity attribute can be formatted either as an integer or using the provided callback with the
/// following signature: `auto(int, writer_t&) -> void` where the first argument means an actual
/// severity level, the second one - streamed writer.
///
/// Process attribute can be represented as either an PID or process name using `:d` and `:s` types
/// respectively: `{process:s}` and `{process:d}`.
///
/// At last the thread attribute can be formatted as either thread id in platform-independent hex
/// representation by default or explicitly with `:s` type, thread id in platform-dependent way
/// using `:d` type or as a thread name if specified, nil otherwise.
///
/// The formatter will throw an exception if an attribute name specified in pattern won't be found
/// in the log record. Of course Blackhole catches this, but it results in dropping the entire
/// message.
/// To avoid this the formatter supports optional generic attributes, which can be specified using
/// the `optional_t` option with an optional prefix and suffix literals printed if an attribute
/// exists.
/// For example an `{id}` pattern with the `[` prefix and `]` suffix options results in empty
/// message if there is no `source` attribute in the record, `[42]` otherwise (where id = 42).
///
/// Blackhole also supports the leftover placeholder starting with `...` and meaning to print all
/// userspace attributes in a reverse order they were provided.
/// These kind of attributes can be configured using special syntax, similar with the timestamp
/// attribute with an optional separator.
/// For example the following placeholder `{...:{{name}={value}:p}{ :s}u}` results in whitespace
/// separated key-value pairs like `id=42` with a special unique-filtered type.
///
/// # Performance
///
/// Internally the given pattern is compiled into the list of tokens during construction time. All
/// further operations are performed using that list to achieve maximum performance.
///
/// All visited tokens are written directly into the given writer instance with an internal small
/// stack-allocated buffer, growing using the heap on overflow.
class string_t;

}  // namespace formatter

template<>
class builder<formatter::string_t> {
public:
    class inner_t;

public:
    explicit builder(std::string pattern);

    auto mapping(formatter::severity_map sevmap) & -> builder&;
    auto mapping(formatter::severity_map sevmap) && -> builder&&;

    auto build() && -> std::unique_ptr<formatter_t>;

private:
    std::unique_ptr<inner_t, deleter_t> p;
};

template<>
class factory<formatter::string_t> : public factory<formatter_t> {
public:
    auto type() const noexcept -> const char* override final;
    auto from(const config::node_t& config) const -> std::unique_ptr<formatter_t> override final;
};

}  // namespace v1
}  // namespace blackhole
