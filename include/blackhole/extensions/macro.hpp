#pragma once

#include "facade.hpp"

#define BLACKHOLE_V1_LOG(__logger__, __sev__, ...) \
    ::blackhole::v1::make_facade(__logger__).log(__sev__, __VA_ARGS__)
