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

struct lazy_message_t {
    /// Initial unformatted message pattern.
    string_view pattern;

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
    std::function<auto() -> string_view> supplier;
};

/// Represents the common logging interface in the library.
///
// TODO: Brief introduction why interface is so, when each method is used and when and how it
//     should be implemented.
/// The library introduces severity levels as an integer number which real meaning depends on the
/// context of your application.
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
// TODO: Wrap severity with class and provide both implicit constructor and operator int().
class logger_t {
public:
    typedef int severity_t;

public:
    virtual ~logger_t() = 0;

    /// Logs the given message with the specified severity level.
    virtual auto log(severity_t severity, const string_view& message) -> void = 0;

    /// Logs the given message with the specified severity level and attributes pack attached.
    virtual auto log(severity_t severity, const string_view& message, attribute_pack& pack) -> void = 0;

    /// Logs the given message with the specified severity level, attributes pack attached and with
    /// special message supplier callback.
    virtual auto log(severity_t severity, const lazy_message_t& message, attribute_pack& pack) -> void = 0;

    /// Attaches the given attributes to the logger, making every further log event to contain them
    /// until returned scoped guard keeped alive.
    virtual auto context() -> boost::thread_specific_ptr<scoped_t>* = 0;
};

}  // namespace v1
}  // namespace blackhole
