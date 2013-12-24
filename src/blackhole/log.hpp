#pragma once

#include "logger.hpp"
#include "utils/format.hpp"

namespace blackhole {

namespace aux {

template<typename Log>
class scoped_pump {
    const Log& log;
    log::record_t& record;

public:
    template<typename... Args>
    scoped_pump(const Log& log, log::record_t& record, Args&&... args) :
        log(log),
        record(record)
    {
        record.attributes["message"] = { blackhole::utils::format(std::forward<Args>(args)...) };
    }

    ~scoped_pump() {
        log.push(std::move(record));
    }

    template<typename... Args>
    void operator ()(Args&&... args) {
        record.fill(std::forward<Args>(args)...);
    }
};

template<typename Log, typename... Args>
scoped_pump<Log> make_scoped_pump(Log& log, log::record_t& record, Args&&... args) {
    return scoped_pump<Log>(log, record, std::forward<Args>(args)...);
}

} // namespace aux

} // namespace blackhole

#define BH_LOG(log, level, ...) \
    for (blackhole::log::record_t record = log.open_record(level); record.valid(); record.attributes.clear()) \
        blackhole::aux::make_scoped_pump(log, record, __VA_ARGS__)
