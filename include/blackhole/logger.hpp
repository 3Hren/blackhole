#pragma once

#include "blackhole/attributes.hpp"
#include "blackhole/message.hpp"
#include "blackhole/severity.hpp"

namespace blackhole {
inline namespace v1 {
namespace scope {

class manager_t;

}  // namespace scope
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {

/// Represents the common logging interface in the library.
///
// TODO: Brief introduction why interface is so, when each method is used and when and how they
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
class logger_t {
public:
    virtual ~logger_t() = 0;

    /// Logs the given message with the specified severity level.
    virtual auto log(severity_t severity, const message_t& message) -> void = 0;

    /// Logs the given message with the specified severity level and attributes pack attached.
    virtual auto log(severity_t severity, const message_t& message, attribute_pack& pack) -> void = 0;

    /// Logs a message which is only to be constructed if the result record passes filtering with
    /// the specified severity and including the attributes pack provided.
    virtual auto log(severity_t severity, const lazy_message_t& message, attribute_pack& pack) -> void = 0;

    /// Returns a scoped attributes manager reference.
    ///
    /// Returned manager allows the external tools to attach scoped attributes to the current logger
    /// instance, making every further log event to contain them until the registered scoped guard
    /// keeped alive.
    ///
    /// \returns a scoped attributes manager.
    virtual auto manager() -> scope::manager_t& = 0;
};

}  // namespace v1
}  // namespace blackhole
