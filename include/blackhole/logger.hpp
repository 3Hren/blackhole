#pragma once

#include <functional>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace fmt {

// TODO: Hide over own class.
template<typename Char, typename Allocator> class BasicMemoryWriter;
typedef BasicMemoryWriter<char, std::allocator<char>> MemoryWriter;

}  // namespace fmt

namespace blackhole {

namespace cppformat = fmt;

class handler_t;

class logger_t {
public:
    typedef std::function<auto(cppformat::MemoryWriter&) -> void> format_t;

public:
    virtual ~logger_t() {}

    virtual auto log(int severity, string_view format) const -> void = 0;
    virtual auto log(int severity, string_view format, range_t& range) const -> void = 0;
    virtual auto log(int severity, string_view format, range_t& range, const format_t& fn) const -> void = 0;
};

}  // namespace blackhole
