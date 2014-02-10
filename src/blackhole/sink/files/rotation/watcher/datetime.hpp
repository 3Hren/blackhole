#pragma once

#include "blackhole/sink/files/rotation/watcher/config.hpp"

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
    typedef typename config_t<datetime_t<TimePicker>>::period_t period_t;

    TimePicker picker;
    period_t period;
    int previous;

    datetime_t(period_t period = period_t::daily) :
        period(period),
        previous(watch())
    {}

    datetime_t(const std::string& period) :
        period(make_period(period)),
        previous(watch())
    {}

    template<typename Backend>
    bool operator ()(Backend&, const std::string&) {
        int current = watch();
        if (current != previous) {
            previous = current;
            return true;
        }

        return false;
    }

private:
    int watch() const {
        const std::tm& timeinfo = picker.now();

        switch (period) {
        case period_t::hourly:
            return timeinfo.tm_hour;
        case period_t::daily:
            return timeinfo.tm_mday;
        case period_t::weekly:
            return timeinfo.tm_wday;
        case period_t::monthly:
            return timeinfo.tm_mon;
        default:
            break;
        }

        return timeinfo.tm_mday;
    }

    static period_t make_period(const std::string& period) {
        if (period == "H") {
            return period_t::hourly;
        } else if (period == "d") {
            return period_t::daily;
        } else if (period == "w") {
            return period_t::weekly;
        } else if (period == "M") {
            return period_t::monthly;
        }

        return period_t::daily;
    }
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
