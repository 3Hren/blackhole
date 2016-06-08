#pragma once

#include <iosfwd>

namespace blackhole {
inline namespace v1 {

/// Terminal color manipulator.
class termcolor_t {
    int attr;
    int code;

public:
    /// Constructs a default terminal color.
    termcolor_t() noexcept;

    /// Creates a terminal color stream manipulator, which equals the blue color.
    ///
    /// Usually useful for marking information or debug events.
    static auto blue() -> termcolor_t;

    /// Creates a terminal color stream manipulator, which equals the green color.
    ///
    /// Usually useful for marking information events.
    static auto green() -> termcolor_t;

    /// Creates a terminal color stream manipulator, which equals the yellow color.
    ///
    /// Usually useful for marking warning events.
    static auto yellow() -> termcolor_t;

    /// Creates a terminal color stream manipulator, which equals the red color.
    ///
    /// Usually useful for marking error events.
    static auto red() -> termcolor_t;

    /// Creates a terminal color stream manipulator, which resets the color.
    static auto reset() -> termcolor_t;

    /// Lightens the current color.
    auto lighter() -> termcolor_t&;

    /// Applies the current color to the given stream and writes data specified to it.
    // TODO: Bad design, subject for change.
    auto apply(std::ostream& stream, const char* data, std::size_t size) -> void;

    /// Checks whether this terminal color differs from the default one.
    auto colored() const noexcept -> bool;

    auto operator==(const termcolor_t& other) const noexcept -> bool;
    auto operator!=(const termcolor_t& other) const noexcept -> bool;

    friend auto operator<<(std::ostream& stream, const termcolor_t& color) -> std::ostream&;

private:
    termcolor_t(int attr, int code) noexcept;
};

} // namespace v1
} // namespace blackhole
