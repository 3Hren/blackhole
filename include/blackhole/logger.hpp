#pragma once

#include <functional>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

class scoped_t;
class writer_t;

class logger_t {
public:
    typedef std::function<auto(writer_t&) -> void> format_t;

public:
    virtual ~logger_t();

    virtual auto log(int severity, string_view pattern) -> void = 0;
    virtual auto log(int severity, string_view pattern, attribute_pack& pack) -> void = 0;
    virtual auto log(int severity, string_view pattern, attribute_pack& pack, const format_t& fn) -> void = 0;

    /// Attaches the given attributes to the logger, making every further log event to contain them
    /// until returned scoped guard keeped alive.
    ///
    /// Calling this method multiple times results in attributes stacking.
    ///
    /// \returns scoped guard which will detach attributes on its destruction.
    virtual auto scoped(attributes_t attributes) -> scoped_t;
};

}  // namespace blackhole
