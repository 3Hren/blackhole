#pragma once

#include "blackhole/keyword.hpp"

namespace blackhole {

DECLARE_UNIVERSE_KEYWORD(pid, std::uint32_t)

namespace keyword {

namespace init {

static inline pid_t pid() {
    static const pid_t pid = ::getpid();
    return pid;
}

} // namespace init

} // namespace keyword

} // namespace blackhole
