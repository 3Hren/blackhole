#pragma once

#include <functional>

#include "blackhole/stdext/string_view.hpp"

namespace blackhole {
inline namespace v1 {

/// Represents a message that can be logged.
typedef string_view message_t;

/// Represents a message that can be logged and can be instantiated lazily on demand.
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

}  // namespace v1
}  // namespace blackhole
