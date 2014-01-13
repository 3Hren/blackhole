#pragma once

#include "formatter.hpp"
#include "sink.hpp"

namespace blackhole {

struct frontend_config_t {
    formatter_config_t formatter;
    sink_config_t sink;
};

} // namespace blackhole
