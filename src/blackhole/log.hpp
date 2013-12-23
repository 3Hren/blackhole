#pragma once

#include "logger.hpp"

#define BH_LOG(log, level, ...) \
    do { \
        blackhole::log::record_t record = log.open_record(level); \
        if (record.valid()) { \
            record.attributes["message"] = { utils::format(__VA_ARGS__) }; \
            log.push(std::move(record)); \
        } \
    } while (0)
