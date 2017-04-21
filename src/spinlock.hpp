#pragma once

#ifdef __linux__
#   include "spinlock.linux.hpp"
#elif __APPLE__
#   include "spinlock.osx.hpp"
#endif
