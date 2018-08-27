#pragma once

#include "blackhole/factory.hpp"

namespace blackhole {
inline namespace v1 {
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

template<>
class builder<sink::asynchronous_t> {
public:
    class inner_t;

private:
    std::unique_ptr<inner_t, deleter_t> d;

public:
    /// Constructs a sink builder from some other sink.
    ///
    /// The ownership of the specified sink is transferred to the builder. The default overflow
    /// policy will block the calling thread while the internal message queue is full.
    ///
    /// \param wrapped The target sink (usually the blocking one) that is need to make asynchronous.
    explicit builder(std::unique_ptr<sink_t> wrapped);

    /// Sets the queue size factor.
    auto factor(std::size_t value) & -> builder&;
    auto factor(std::size_t value) && -> builder&&;

    /// Sets the drop overflow policy.
    auto drop() & -> builder&;
    auto drop() && -> builder&&;

    /// Sets the wait overflow policy.
    auto wait() & -> builder&;
    auto wait() && -> builder&&;

    /// Consumes this builder yielding a newly created asynchronous sink with the options
    /// configured.
    auto build() && -> std::unique_ptr<sink_t>;
};

/// Represents asynchronous sink factory.
///
/// Register an instance of this class in the registry to be able to construct asynchronous sinks
/// via config.
template<>
class factory<sink::asynchronous_t> : public factory<sink_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace v1
}  // namespace blackhole
