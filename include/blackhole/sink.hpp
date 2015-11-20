#pragma once

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

using cpp17::string_view;

class record_t;

class sink_t {
    virtual ~sink_t() {}

    virtual auto filter(const record_t& record) -> bool = 0;

    virtual auto execute(const record_t& record, const string_view& formatted) -> void = 0;
};

}  // namespace blackhole
