#pragma once

#include "blackhole/config.hpp"

#include "log.hpp"

BLACKHOLE_BEG_NS

namespace repository {

namespace config {

static inline log_config_t trivial() {
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    sink_config_t sink("stream");
    sink["output"] = "stdout";

    frontend_config_t frontend = { formatter, sink };
    return log_config_t{ "root", { frontend } };
}

} // namespace config

} // namespace repository

BLACKHOLE_END_NS
