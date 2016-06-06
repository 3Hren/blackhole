#pragma once

#include <functional>

#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {

template<typename>
struct factory;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace sink {

/// Terminal color manipulator.
class color_t {
    class scope_t;

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

typedef std::function<auto(const record_t&) -> color_t> termcolor_map;

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
class console_t : public sink_t {
public:
    enum class type_t { stdout_v, stderr_v };

private:
    std::ostream& stream;
    termcolor_map colormap;

public:
    class builder_t;

    /// Constructs a default console sink, which will write all incoming events to the standard
    /// output.
    ///
    /// No output coloring occurred in this case.
    console_t();

    /// Constucts a new console sink, which will write all incoming events to the standard output
    /// using the given terminal color mapping to colorize the output.
    ///
    /// \param colormap color mappings from logging level to terminal color.
    explicit console_t(termcolor_map colormap);

    /// Constructs a new console sink, which will write all incoming events to the specified
    /// standard stream (stdout or stderr) using the given terminal color mapping to colorize the
    /// output.
    ///
    /// \param type standard output type.
    /// \param colormap color mappings from logging level to terminal color.
    console_t(type_t type, termcolor_map colormap);

    /// Writes the formatted message into the attached output stream.
    ///
    /// Note that the message may be anticipatorily colored using severity information from the
    /// associated record.
    auto emit(const record_t& record, const string_view& formatted) -> void;

protected:
    /// Constructs a new console sink, which will write all incoming events to the specified
    /// stream using the given terminal color mapping to colorize the output.
    ///
    /// This constructor is protected, because it accepts a generic stream instead of predefined
    /// one making it possible to write into anything that implements stream protocol, which is
    /// useful for testing reasons for example.
    console_t(std::ostream& stream, termcolor_map colormap);

    /// Maps the given log record returning the color that is used for colored formatting the result
    /// output message.
    ///
    /// The implementation uses `colormap` variable provided with construction.
    virtual auto color(const record_t& record) const -> color_t;

private:
    static auto output(type_t type) -> std::ostream&;
};

class console_t::builder_t {
public:
    auto build() const -> console_t;
};

}  // namespace sink

template<>
struct factory<sink::console_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::console_t;
};

}  // namespace v1
}  // namespace blackhole
