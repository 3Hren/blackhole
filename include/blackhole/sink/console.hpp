#pragma once

#include <functional>

#include "blackhole/sink.hpp"

namespace blackhole {

class config_t;

template<typename>
struct factory;

}  // namespace blackhole

namespace blackhole {
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

class console_t : public sink_t {
public:
    enum class type_t { stdout, stderr };

private:
    std::ostream& stream;
    termcolor_map colormap;

public:
    console_t();
    explicit console_t(termcolor_map colormap);
    console_t(type_t type, termcolor_map colormap);

    auto filter(const record_t& record) -> bool;
    auto execute(const record_t& record, const string_view& formatted) -> void;

protected:
    console_t(std::ostream& stream, termcolor_map colormap);

    auto color(const record_t& record) -> color_t;

private:
    static auto output(type_t type) -> std::ostream&;
};

}  // namespace sink

template<>
struct factory<sink::console_t> {
    static auto type() -> const char*;
    static auto from(const config_t& config) -> sink::console_t;
};

}  // namespace blackhole
