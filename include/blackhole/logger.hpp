#pragma once

#include <functional>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

class writer_t;
class handler_t;

class logger_t {
public:
    typedef std::function<auto(writer_t&) -> void> format_t;

public:
    virtual ~logger_t() {}

    virtual auto log(int severity, string_view format) const -> void = 0;
    virtual auto log(int severity, string_view format, range_t& range) const -> void = 0;
    virtual auto log(int severity, string_view format, range_t& range, const format_t& fn) const -> void = 0;
};

}  // namespace blackhole
