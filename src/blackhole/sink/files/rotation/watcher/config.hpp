#pragma once

#include <cstdint>
#include <string>

#include <boost/mpl/set.hpp>

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

struct size_t;
template<class> struct datetime_t;
template<class...> struct watcher_set;

template<class Watcher>
struct config_t;

template<>
struct config_t<watcher::size_t> {
    std::uint64_t size;

    config_t(std::uint64_t size = 1 * 1024 * 1024) :
        size(size)
    {}
};

template<class TimePicker>
struct config_t<watcher::datetime_t<TimePicker>> {
    enum class period_t {
        hourly,
        daily,
        weekly,
        monthly
    };

    std::string period;
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
