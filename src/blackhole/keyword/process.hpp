#pragma once

#include "blackhole/keyword.hpp"

BLACKHOLE_BEG_NS

DECLARE_UNIVERSE_KEYWORD(pid, std::uint32_t)

namespace keyword {

namespace init {

static inline pid_t pid() {
    static const pid_t pid = ::getpid();
    return pid;
}

} // namespace init

} // namespace keyword

BLACKHOLE_END_NS
