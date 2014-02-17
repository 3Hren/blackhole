#pragma once

#include <string>

#include "blackhole/keyword.hpp"
#include "blackhole/utils/thread.hpp"

namespace blackhole { namespace keyword {
DECLARE_THREAD_KEYWORD(tid, std::string)
} }
