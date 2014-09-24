#pragma once

#include <cstdint>
#include <string>

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

struct size_t;
struct move_t;
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

template<>
struct config_t<move_t> {};

namespace datetime {

enum class period_t {
    hourly,
    daily,
    weekly,
    monthly
};

} // namespace datetime

template<class TimePicker>
struct config_t<watcher::datetime_t<TimePicker>> {
    std::string period;
};

template<class... Args>
struct config_t<watcher_set<Args...>> {
    std::uint64_t size;
    std::string period;
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
