#pragma once

#include "logger.hpp"

#define BH_LOG(log, level, message, ...) \
    do { \
        blackhole::log::record_t record = log.open_record(level); \
        if (record.valid()) { \
            record.attributes["message"] = { utils::format(message, __VA_ARGS__) }; \
            log.push(std::move(record)); \
        } \
    } while (0)
