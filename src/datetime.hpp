#pragma once

#ifdef __linux__
#include "datetime/generator.linux.hpp"
#else
#include "datetime/generator.other.hpp"
#endif
