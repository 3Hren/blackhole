#pragma once

#include "blackhole/logger.hpp"
#include "blackhole/detail/logger/pusher.hpp"

inline
bool
__attribute__((always_inline))
__attribute__((format(__printf__, 1, 2)))
check(const char*, ...) {
    return true;
}

#define BH_LOG(__log__, __level__, ...) \
    if (auto record = (__log__).open_record((__level__))) \
        if (check(__VA_ARGS__)) \
            blackhole::aux::logger::make_pusher((__log__), record, __VA_ARGS__)
