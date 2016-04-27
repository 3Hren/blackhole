#pragma once

#include <memory>
#include <functional>

#include "blackhole/forward.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

namespace syslog {

/// Transforms the given severity to the corresponding syslog priority value.
typedef std::function<auto(severity_t severity) -> int> priority_t;

/// Represents system logger backend.
class backend_t {
public:
    virtual ~backend_t() = 0;

    /// Opens or reopens an internal connection to the system logger.
    ///
    /// Depending on the implementation the connection may be program wide in the case of native
    /// backend.
    virtual auto open() -> void = 0;

    /// Closes the internal connection to the system logger.
    ///
    /// Should have no effect if the connection is already closed.
    virtual auto close() noexcept -> void = 0;

    /// Writes the given logging message with the associated priority into the syslog pipe.
    virtual auto write(int priority, const string_view& message) -> void = 0;
};

}  // namespace syslog

/// Represents a system logger sink interface, that send messages to the system logger.
class syslog_t;

}  // namespace sink

template<>
struct factory<sink::syslog_t> {
    typedef sink::syslog_t sink_type;
    typedef sink::syslog::backend_t backend_t;
    typedef sink::syslog::priority_t priority_t;

    /// Returns this sink factory type identifier.
    static auto type() -> const char*;

    /// Constructs and returns a new syslog sink from the specified configuration.
    static auto from(const config::node_t& config) -> std::unique_ptr<sink_type>;

    /// Constructs and returns a new syslog sink by injecting the specified dependencies.
    static auto construct(std::unique_ptr<backend_t> backend, priority_t priority) ->
        std::unique_ptr<sink_type>;

    /// Constructs and returns a new native syslog backend.
    ///
    /// \param identity is an arbitrary identification string which future syslog invocations will
    ///     prefix to each message. This is intended to identify the source of the message, and
    ///     people conventionally set it to the name of the program that will submit the messages.
    ///     If the identity is not set, the default identification string will be the program name.
    /// \param option is a bit string, with the bits as defined in POSIX.1-2001, and POSIX.1-2008.
    ///     The default value is LOG_PID.
    /// \param facility is used to specify what type of program is logging the message. This lets
    ///     the configuration file specify that messages from different facilities will be handled
    ///     differently. The default value is LOG_USER.
    static auto native_backend(const char* identity, int option = 0, int facility = 0) ->
        std::unique_ptr<backend_t>;
};

}  // namespace v1
}  // namespace blackhole
