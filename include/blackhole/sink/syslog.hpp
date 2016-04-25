#pragma once

#include <syslog.h>

#include <memory>
#include <vector>

#include "blackhole/forward.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

/// parse ident, option, facility.
/// construct;
/// connect(...);


/// A system logger sink send messages to the system logger.
class syslog_t : public sink_t {
public:
    /// Emits the given log record with the corresponding formatted message into the syslog pipe.
    virtual auto emit(const record_t& record, const string_view& message) -> void;

    /// Transforms the given severity to the corresponding syslog priority value.
    ///
    /// The method is called each time the new logging record is emitted by this sink. Override this
    /// if you want more specific mapping between logger severity and syslog priority.
    virtual auto priority(severity_t severity) const noexcept -> int = 0;
};

}  // namespace sink

template<>
struct factory<sink::syslog_t> {
    static auto type() -> const char*;

    /// The identity is an arbitrary identification string which future syslog invocations will prefix
    /// to each message. This is intended to identify the source of the message, and people
    /// conventionally set it to the name of the program that will submit the messages.
    /// If the ident is  not set, the default identification string will be the program name.
    ///
    /// The facility is used to specify what type of program is logging the message. This lets the
    /// configuration file specify that messages from different facilities will be handled differently.
    /// The default value is LOG_USER.
    ///
    /// The option is a bit string, with the bits as defined in POSIX.1-2001, and POSIX.1-2008. The
    /// default value is LOG_PID.
    static auto from(const config::node_t& config) -> std::unique_ptr<sink::syslog_t>;
};

}  // namespace v1
}  // namespace blackhole
