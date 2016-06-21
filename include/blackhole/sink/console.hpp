#pragma once

#include <functional>

#include "blackhole/factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

/// Represents the console sink which is responsible for writing all incoming log events directly
/// into one of the selected standard outputs with an ability to optionally colorize result strings.
///
/// The sink automatically detects whether the destination stream is a TTY disabling colored output
/// otherwise, which makes possible to redirect standard output to file without escaping codes
/// garbage.
///
/// Note, that despite of C++ `std::cout` and `std::cerr` thread-safety with no undefined behavior
/// its guarantees is insufficiently for safe working with them from multiple threads, leading to
/// result messages intermixing.
/// To avoid this a global mutex is used internally, which is kinda hack. Any other stdout/stderr
/// usage outside from logger will probably results in character mixing, but no undefined behavior
/// will be invoked.
class console_t;

}  // namespace sink

template<>
class builder<sink::console_t> {
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> d;

public:
    /// Constructs a defaultly configured console sink builder.
    ///
    /// By default the generated sink will write all incoming events to the standard output with no
    /// coloring.
    builder();

    /// Sets the destination stream to the standard output pipe.
    auto stdout() & -> builder&;
    auto stdout() && -> builder&&;

    /// Sets the destination stream to the standard error pipe.
    auto stderr() & -> builder&;
    auto stderr() && -> builder&&;

    /// Sets terminal color mapping for a given severity making all log events to be colored with
    /// specified color.
    auto colorize(severity_t severity, termcolor_t color) & -> builder&;
    auto colorize(severity_t severity, termcolor_t color) && -> builder&&;

    /// Resets the terminal color mapping for this builder with the specified one.
    auto colorize(std::function<termcolor_t(const record_t& record)> fn) & -> builder&;
    auto colorize(std::function<termcolor_t(const record_t& record)> fn) && -> builder&&;

    /// Consumes this builder yielding a newly created console sink with the options configured.
    auto build() && -> std::unique_ptr<sink_t>;
};

template<>
class factory<sink::console_t> : public factory<sink_t> {
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
