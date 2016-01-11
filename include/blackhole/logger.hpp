#pragma once

#include <functional>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace boost {
    template<typename> class thread_specific_ptr;
}  // namespace boost

namespace blackhole {
inline namespace v1 {

class scoped_t;

/// Represents the common logging interface in the library.
///
/// # Severity
///
/// The library introduces severity levels as an integer number which real meaning depends on the
/// context of your application. Usually there are about 4-5 severity levels and it's important to
/// understand that every logging event contains an information with some severity level and the
/// reaction time for that event differs.
/// For example:
/// - Debug: "parsing HTTP request"
/// - Debug: "accepted new TCP connection"
/// - Info : "service Elasticsearch has been exposed on 9200 port"
/// - Warn : "unable to enqueue event with no name"
/// - Error: "unable to process proxy request: proxy is down"
/// - Fatal: "unable to load the MQ module: permission denied"
///
/// # Construction
///
/// Blackhole provides two convenient ways to obtain a configured logger instance.
///
/// If you want to configure a logger programmically, there are typed builder classes that provide
/// chained interface for simplifying and making the code eye-candy.
///
/// On the other hand sometimes you can't build a logger programmically, because its configuration
/// is unknown at compile time and is obtained from some config file for example. Or if you have a
/// configuration object (JSON, XML, folly dynamic) you may want to create a logger using it. For
/// these cases there is a \sa registry_t class.
///
/// Otherwise you can always build logger instances directly using its constructors.
class logger_t {
public:
    /// Message supplier callback, that should be called only when the logging event passes
    /// filtering to obtain the final formatted message.
    ///
    /// Sometimes obtaining a logging message requires heavyweight operation to be performed and
    /// there is no guarantee that the result of this operation won't be immediately dropped because
    /// of failed filtering for example. For these cases we allow the client code to provide
    /// messages lazily.
    /// Note, that string view semantics requires the real message storage that is pointed by view
    /// to outlive the function call. Default implementation in the facade just allocates large
    /// buffer on stack and fills it on function invocation.
    typedef std::function<auto() -> string_view> supplier_t;

public:
    virtual ~logger_t() = 0;

    /// Logs the given message with the specified severity level.
    virtual auto log(int severity, const string_view& message) -> void = 0;

    /// Logs the given message with the specified severity level and attributes pack attached.
    virtual auto log(int severity, const string_view& message, attribute_pack& pack) -> void = 0;

    /// Logs the given message with the specified severity level, attributes pack attached and with
    /// special message supplier callback.
    virtual auto log(int severity, const string_view& message, attribute_pack& pack, const supplier_t& supplier) -> void = 0;

    /// Attaches the given attributes to the logger, making every further log event to contain them
    /// until returned scoped guard keeped alive.
    virtual auto context() -> boost::thread_specific_ptr<scoped_t>* = 0;
};

}  // namespace v1
}  // namespace blackhole
