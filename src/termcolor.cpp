#include <ostream>

#include "blackhole/termcolor.hpp"

namespace blackhole {
inline namespace v1 {
namespace {

class scope_t {
    std::ostream& stream;

public:
    scope_t(std::ostream& stream, const termcolor_t& color) :
        stream(stream)
    {
        stream << color;
    }

    ~scope_t() noexcept(false) {
        stream << termcolor_t::reset();
    }
};

}  // namespace

termcolor_t::termcolor_t() noexcept :
    attr(0),
    code(39)
{}

termcolor_t::termcolor_t(int attr, int code) noexcept :
    attr(attr),
    code(code)
{}

auto termcolor_t::gray() -> termcolor_t {
    return {0, 2};
}

auto termcolor_t::blue() -> termcolor_t {
    return termcolor_t(0, 34);
}

auto termcolor_t::green() -> termcolor_t {
    return termcolor_t(0, 32);
}

auto termcolor_t::yellow() -> termcolor_t {
    return termcolor_t(0, 33);
}

auto termcolor_t::red() -> termcolor_t {
    return termcolor_t(0, 31);
}

auto termcolor_t::reset() -> termcolor_t {
    return termcolor_t(0, 0);
}

auto termcolor_t::operator==(const termcolor_t& other) const noexcept -> bool {
    return code == other.code && attr == other.attr;
}

auto termcolor_t::operator!=(const termcolor_t& other) const noexcept -> bool {
    return !operator==(other);
}

auto termcolor_t::write(std::ostream& stream, const char* data, std::size_t size) -> void {
    if (colored()) {
        scope_t scope(stream, *this);
        stream.write(data, static_cast<std::streamsize>(size));
    } else {
        stream.write(data, static_cast<std::streamsize>(size));
    }
}

auto termcolor_t::colored() const noexcept -> bool {
    return *this != termcolor_t();
}

auto operator<<(std::ostream& stream, const termcolor_t& color) -> std::ostream& {
    return stream << "\033[" << color.code << "m";
}

}  // namespace v1
}  // namespace blackhole
