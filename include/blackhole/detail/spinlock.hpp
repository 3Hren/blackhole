#pragma once

#ifdef __linux__
#   include "blackhole/detail/spinlock.linux.hpp"
#elif __APPLE__
#   include "blackhole/detail/spinlock.osx.hpp"
#endif
