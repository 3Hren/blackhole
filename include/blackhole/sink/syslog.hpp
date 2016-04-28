#pragma once

#include <memory>
#include <functional>

#include "blackhole/factory.hpp"
#include "blackhole/forward.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

namespace syslog {

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

/// Transforms the given severity to the corresponding syslog priority value.
typedef std::function<auto(severity_t severity) -> int> priority_map;

}  // namespace syslog

/// Represents a system logger sink interface, that send messages to the system logger.
class syslog_t : public sink_t {
public:
    /// Returns an option a bit fields, that is used by current sink instance.
    ///
    /// \note depending on the implementation this value can be changed at runtime. For example,
    ///     native backend works this way.
    virtual auto option() const -> int = 0;

    /// Returns a facility value that is used to specify what type of program is logging the
    /// message.
    ///
    /// \note depending on the implementation this value can be changed at runtime. For example,
    ///     native backend works this way.
    virtual auto facility() const -> int = 0;

    /// Returns an identity string litral which future syslog invocations will prefix to each
    /// message.
    ///
    /// \note depending on the implementation this value can be changed at runtime. For example,
    ///     native backend works this way.
    virtual auto identity() const -> const char* = 0;

    /// Transforms the given severity to the corresponding syslog priority value.
    ///
    /// \param severity is an event severity.
    virtual auto priority(severity_t severity) const -> int = 0;
};

}  // namespace sink

template<>
struct factory<sink::syslog_t> : public factory<sink_t> {
    typedef sink::syslog_t syslog_t;
    typedef sink::syslog::backend_t backend_t;
    typedef sink::syslog::priority_map priority_map;

    /// Returns factory type as a string literal, that is used to identity one factory of a given
    /// type from another.
    virtual auto type() const noexcept -> const char* override;

    /// Constructs and returns a new syslog sink from the specified configuration.
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;

    /// Constructs and returns a new syslog sink by injecting the specified dependencies.
    static auto construct(std::unique_ptr<backend_t> backend, priority_map priomap) ->
        std::unique_ptr<syslog_t>;

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
