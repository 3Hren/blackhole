#include "blackhole/sink/console.hpp"

#include <iostream>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
namespace sink {

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

auto get_standard_stream(const std::ostream& stream) -> FILE* {
    return nullptr;
}

auto isatty(const std::ostream& stream) -> bool {
    return true;
}

}  // namespace

console_t::console_t() :
    colormap([](const record_t&) -> color_t { return {}; })
{}

console_t::console_t(color_map colormap) :
    colormap(std::move(colormap))
{}

auto console_t::filter(const record_t&) -> bool {
    return true;
}

auto console_t::execute(const record_t& record, const string_view& formatted) -> void {
    if (isatty(std::cout)) {
        // std::lock_guard<std::mutex> lock(mutex);
        color(record).apply(std::cout, formatted.data(), formatted.size());
        std::cout << std::endl;
    } else {
        // std::lock_guard<std::mutex> lock(mutex);
        std::cout.write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
        std::cout << std::endl;
    }
}

auto console_t::color(const record_t& record) -> color_t {
    return colormap(record);
}

}  // namespace sink

auto
factory<sink::console_t>::type() -> const char* {
    return "console";
}

auto
factory<sink::console_t>::from(const config_t&) -> sink::console_t {
    return {};
}

}  // namespace blackhole
