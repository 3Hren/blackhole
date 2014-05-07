#pragma once

#include <blackhole/log.hpp>
#include <blackhole/synchronized.hpp>

#define LOG(__log__, ...) \
    if (blackhole::log::record_t record = __log__.open_record()) \
        blackhole::aux::make_scoped_pump(__log__, record, __VA_ARGS__)
