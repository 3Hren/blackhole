#pragma once

#include <ctime>

#include "blackhole/sink/files/rotation/watcher/config.hpp"
#include "blackhole/utils/nullptr.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

struct time_picker_t {
    inline std::tm now() const {
        std::time_t time = std::time(nullptr);
        std::tm timeinfo;
        ::localtime_r(&time, &timeinfo);
        return timeinfo;
    }
};

template<class TimePicker = time_picker_t>
struct datetime_t {
    static const char* name() {
        return "datetime";
    }

    TimePicker picker;
    const datetime::period_t period;

    datetime_t(datetime::period_t period = datetime::period_t::daily) :
        period(period),
        previous(watch())
    {}

    datetime_t(const std::string& period) :
        period(period_factory_t::get(period)),
        previous(watch())
    {}

    template<class T>
    datetime_t(const config_t<T>& config) :
        period(period_factory_t::get(config.period)),
        previous(watch())
    {}

    template<typename Backend>
    bool operator ()(Backend&, const std::string&) const {
        int current = watch();
        if (current != previous) {
            previous = current;
            return true;
        }

        return false;
    }

private:
    mutable int previous;

    int watch() const {
        const std::tm& timeinfo = picker.now();

        switch (period) {
        case datetime::period_t::hourly:
            return timeinfo.tm_hour;
        case datetime::period_t::daily:
            return timeinfo.tm_mday;
        case datetime::period_t::weekly:
            return timeinfo.tm_wday;
        case datetime::period_t::monthly:
            return timeinfo.tm_mon;
        default:
            break;
        }

        return timeinfo.tm_mday;
    }

    class period_factory_t {
    public:
        static datetime::period_t get(const std::string& period) {
            if (period == "H") {
                return datetime::period_t::hourly;
            } else if (period == "d") {
                return datetime::period_t::daily;
            } else if (period == "w") {
                return datetime::period_t::weekly;
            } else if (period == "M") {
                return datetime::period_t::monthly;
            }

            return datetime::period_t::daily;
        }
    };
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
