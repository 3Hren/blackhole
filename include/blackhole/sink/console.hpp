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

namespace experimental {

/// Terminal color manipulator.
class color_t {
    int attr;
    int code;

public:
    /// Constructs a default terminal color.
    color_t();

    /// Creates a terminal color stream manipulator, which equals the blue color.
    ///
    /// Usually useful for marking information events.
    static auto blue() -> color_t;

    /// Creates a terminal color stream manipulator, which equals the green color.
    ///
    /// Usually useful for marking information events.
    static auto green() -> color_t;

    /// Creates a terminal color stream manipulator, which equals the yellow color.
    ///
    /// Usually useful for marking warning events.
    static auto yellow() -> color_t;

    /// Creates a terminal color stream manipulator, which equals the red color.
    ///
    /// Usually useful for marking error events.
    static auto red() -> color_t;

    /// Creates a terminal color stream manipulator, which resets the color.
    static auto reset() -> color_t;

    /// Lightens the current color.
    auto lighter() -> color_t&;

    /// Applies the current color to the given stream and writes data specified to it.
    auto apply(std::ostream& stream, const char* data, std::size_t size) -> void;

    auto colored() const noexcept -> bool;

    auto operator==(const color_t& other) const noexcept -> bool;
    auto operator!=(const color_t& other) const noexcept -> bool;

    friend auto operator<<(std::ostream& stream, const color_t& color) -> std::ostream&;

private:
    color_t(int attr, int code);
};

template<>
class builder<sink::console_t> {
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

    /// TODO(docs): write me.
    auto colorize(severity_t severity, color_t color) & -> builder&;
    auto colorize(severity_t severity, color_t color) && -> builder&&;

    /// TODO(docs): write me.
    auto colorize(std::function<color_t(const record_t& record)> fn) & -> builder&;
    auto colorize(std::function<color_t(const record_t& record)> fn) && -> builder&&;

    /// TODO(docs): write me.
    auto build() && -> std::unique_ptr<sink_t>;
};

template<>
class factory<sink::console_t> : public factory<sink_t> {
public:
    auto type() const noexcept -> const char*;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t>;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
