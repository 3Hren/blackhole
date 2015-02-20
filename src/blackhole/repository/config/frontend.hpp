#pragma once

#include "blackhole/config.hpp"

#include "formatter.hpp"
#include "sink.hpp"

BLACKHOLE_BEG_NS

struct frontend_config_t {
    formatter_config_t formatter;
    sink_config_t sink;
};

BLACKHOLE_END_NS
