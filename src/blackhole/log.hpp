#pragma once

#include "logger.hpp"
#include "utils/format.hpp"
#include "utils/nullptr.hpp"

namespace blackhole {

template<typename Log>
class pusher_t {
    Log* log;
    blackhole::log::record_t record;

public:
    pusher_t(Log* log = nullptr, blackhole::log::record_t&& record = blackhole::log::record_t()) :
        log(log),
        record(record)
    {}

    template<typename... Args>
    inline void operator ()(Args&&... args) {
        if (log) {
            record.fill(std::forward<Args>(args)...);
            log->push(std::move(record));
            log = nullptr;
        }
    }
};

template<typename Log, typename Level, typename... Args>
inline pusher_t<Log> log_with_attributes(Log& log, Level level, const std::string& message, Args&&... args) {
    blackhole::log::record_t record = log.open_record(level);
    if (record.valid()) {
        record.attributes["message"] = { blackhole::utils::format(message, std::forward<Args>(args)...) };
        return pusher_t<Log>(&log, std::move(record));
    }

    return pusher_t<Log>();
}

} // namespace blackhole

#define BH_LOG(log, level, ...) \
    do { \
        blackhole::log::record_t record = log.open_record(level); \
        if (record.valid()) { \
            record.attributes["message"] = { blackhole::utils::format(__VA_ARGS__) }; \
            log.push(std::move(record)); \
        } \
    } while (0)

#define BH_LOG_WA log_with_attributes
