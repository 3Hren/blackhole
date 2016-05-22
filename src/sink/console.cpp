#include "blackhole/sink/console.hpp"

#include <unistd.h>

#include <iostream>
#include <mutex>

#include "blackhole/cpp17/string_view.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/sink/console.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

using experimental::color_t;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

// Both standard output and error access mutex. Messages written with Blackhole will be
// synchronized, otherwise an intermixing can occur.
static std::mutex mutex;

#pragma clang diagnostic pop

typedef std::function<auto(const record_t& record) -> color_t> termcolor_map;

namespace {

auto streamfd(const std::ostream& stream) noexcept -> FILE* {
    if (&stream == &std::cout) {
        return stdout;
    } else if (&stream == &std::cerr) {
        return stderr;
    } else {
        return nullptr;
    }
}

auto isatty(const std::ostream& stream) -> bool {
    if (auto file = streamfd(stream)) {
#if defined(__linux__) || defined(__APPLE__)
        return ::isatty(::fileno(file));
#else
#error unsupported platform
#endif
    } else {
        return false;
    }
}

}  // namespace


console_t::console_t() :
    stream(std::cout),
    colormap([](const record_t&) -> color_t { return {}; })
{}

/// Constructs a new console sink, which will write all incoming events to the specified
/// stream using the given terminal color mapping to colorize the output.
///
/// This constructor is protected, because it accepts a generic stream instead of predefined
/// one making it possible to write into anything that implements stream protocol, which is
/// useful for testing reasons for example.
console_t::console_t(std::ostream& stream, std::function<auto(const record_t& record) -> color_t> colormap) :
    stream(stream),
    colormap(std::move(colormap))
{}

/// Writes the formatted message into the attached output stream.
///
/// Note that the message may be anticipatorily colored using severity information from the
/// associated record.
auto console_t::emit(const record_t& record, const string_view& formatted) -> void {
    if (isatty(stream)) {
        std::lock_guard<std::mutex> lock(mutex);
        colormap(record)
            .apply(stream, formatted.data(), formatted.size());
        stream << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(mutex);
        stream.write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
        stream << std::endl;
    }
}

}  // namespace sink

namespace experimental {
namespace {

class scope_t {
    std::ostream& stream;

public:
    scope_t(std::ostream& stream, const color_t& color) :
        stream(stream)
    {
        stream << color;
    }

    ~scope_t() noexcept(false) {
        stream << color_t::reset();
    }
};

}  // namespace

color_t::color_t() :
    attr(0),
    code(39)
{}

color_t::color_t(int attr, int code) :
    attr(attr),
    code(code)
{}

auto color_t::blue() -> color_t {
    return color_t(0, 34);
}

auto color_t::green() -> color_t {
    return color_t(0, 32);
}

auto color_t::yellow() -> color_t {
    return color_t(0, 33);
}

auto color_t::red() -> color_t {
    return color_t(0, 31);
}

auto color_t::reset() -> color_t {
    return color_t(0, 0);
}

auto color_t::operator==(const color_t& other) const noexcept -> bool {
    return code == other.code && attr == other.attr;
}

auto color_t::operator!=(const color_t& other) const noexcept -> bool {
    return !operator==(other);
}

auto color_t::apply(std::ostream& stream, const char* data, std::size_t size) -> void {
    if (colored()) {
        scope_t scope(stream, *this);
        stream.write(data, static_cast<std::streamsize>(size));
    } else {
        stream.write(data, static_cast<std::streamsize>(size));
    }
}

auto color_t::colored() const noexcept -> bool {
    return *this != color_t();
}

auto operator<<(std::ostream& stream, const color_t& color) -> std::ostream& {
    return stream << "\033[" << color.code << "m";
}

auto factory<sink::console_t>::type() const noexcept -> const char* {
    return "console";
}

auto factory<sink::console_t>::from(const config::node_t&) const -> std::unique_ptr<sink_t> {
    return blackhole::make_unique<sink::console_t>();
}

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
