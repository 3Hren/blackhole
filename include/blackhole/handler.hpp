#pragma once

#include <memory>

namespace blackhole {
inline namespace v1 {

class record_t;

/// Represents logging handler interface.
class handler_t {
public:
    virtual ~handler_t() = 0;

    /// Handles the given record.
    ///
    /// By handling a record we usually mean doing at least three actions over it: filtering,
    /// formatting and emitting to the targets.
    ///
    /// \warning must be thread-safe.
    virtual auto handle(const record_t& record) -> void = 0;
};

}  // namespace v1
}  // namespace blackhole
