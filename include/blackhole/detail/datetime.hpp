#pragma once

#ifdef __linux__
#include "blackhole/detail/datetime/linux/generator.hpp"
#else
#include "blackhole/detail/datetime/other/generator.hpp"
#endif
