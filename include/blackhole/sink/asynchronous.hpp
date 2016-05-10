#pragma once

#include "blackhole/factory.hpp"
#include "blackhole/forward.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {

/// The asynchronous sink wraps the other sink and causes log events to be written to them on a
/// separate thread.
///
/// Depending on the underlying overflow policies and queues the implementation may preallocate the
/// space required to keep all item structs in the memory.
///
/// \note exceptions while writing to those sink will be hidden from the application.
///
/// # Parameters
///
/// The factor value maps directly into the queue capacity and equals exp2(factor). The value must
/// fit in [0; 20] range (1048576 items).
///
/// Overflow policy decides what action is taken when the queue is overflowed. There are currently
/// only two available policies: drop and wait. The first one will silently (or not) drop all log
/// events that weren't enqueued. The second one will block the caller thread until the queue is
/// full.
///
/// \throw std::invalid_argument on construction if the factor is greater than 20.
/// \throw std::invalid_argument on construction if the overflow policy value differs from "drop" or
///     "wait".
class asynchronous_t;

}  // namespace sink

/// Represents asynchronous sink factory.
///
/// Register an instance of this class in the registry to be able to construct asynchronous sinks
/// via config.
template<>
class factory<sink::asynchronous_t> : public experimental::factory<sink_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    virtual auto type() const noexcept -> const char* override;
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
