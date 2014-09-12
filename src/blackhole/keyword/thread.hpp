#pragma once

#include <string>

#include "blackhole/detail/util/thread.hpp"
#include "blackhole/keyword.hpp"

namespace blackhole {

DECLARE_THREAD_KEYWORD(tid, std::string)
DECLARE_THREAD_KEYWORD(lwp, uint64_t)

} // namespace blackhole
