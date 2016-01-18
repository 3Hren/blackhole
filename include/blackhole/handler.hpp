#pragma once

#include <memory>

namespace blackhole {
inline namespace v1 {

class formatter_t;
class record_t;
class sink_t;

/// Represents logging handler interface.
class handler_t {
public:
    handler_t() = default;
    handler_t(const handler_t& other) = default;
    handler_t(handler_t&& other) = default;

    virtual ~handler_t() = 0;

    /// Handles the given record.
    ///
    /// By handling a record we usually mean doing at least three actions over it: filtering,
    /// formatting and emitting to the targets.
    ///
    /// \warning must be thread-safe.
    virtual auto handle(const record_t& record) -> void = 0;

    virtual auto set(std::unique_ptr<formatter_t> formatter) -> void = 0;
    virtual auto add(std::unique_ptr<sink_t> sink) -> void = 0;
};

}  // namespace v1
}  // namespace blackhole
