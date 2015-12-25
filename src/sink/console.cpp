#include "blackhole/sink/console.hpp"

#include <unistd.h>

#include <iostream>
#include <mutex>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
namespace sink {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

// Both standard output and error access mutex. Messages written with Blackhole will be
// synchronized, otherwise an intermixing can occur.
static std::mutex mutex;

#pragma clang diagnostic pop

class color_t::scope_t {
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

namespace {

auto streamfd(const std::ostream& stream) -> FILE* {
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

console_t::console_t(termcolor_map colormap) :
    stream(std::cout),
    colormap(std::move(colormap))
{}

console_t::console_t(type_t type, termcolor_map colormap) :
    stream(output(type)),
    colormap(std::move(colormap))
{}

console_t::console_t(std::ostream& stream, termcolor_map colormap) :
    stream(stream),
    colormap(std::move(colormap))
{}

auto console_t::filter(const record_t&) -> bool {
    return true;
}

auto console_t::execute(const record_t& record, const string_view& formatted) -> void {
    if (isatty(stream)) {
        std::lock_guard<std::mutex> lock(mutex);
        color(record).apply(stream, formatted.data(), formatted.size());
        stream << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(mutex);
        stream.write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
        stream << std::endl;
    }
}

auto console_t::color(const record_t& record) -> color_t {
    return colormap(record);
}

auto console_t::output(type_t type) -> std::ostream& {
    return type == type_t::stdout ? std::cout : std::cerr;
}

auto console_t::builder_t::build() const -> console_t {
    return {};
}

}  // namespace sink

auto
factory<sink::console_t>::type() -> const char* {
    return "console";
}

auto
factory<sink::console_t>::from(const config::node_t&) -> sink::console_t {
    return {};
}

}  // namespace blackhole
