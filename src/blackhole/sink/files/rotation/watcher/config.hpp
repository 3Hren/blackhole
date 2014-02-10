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

    std::string marker;
};

template<class TimePicker>
struct config_t<boost::mpl::set<watcher::size_t, watcher::datetime_t<TimePicker>>> {
    std::uint64_t size;
    std::string marker;

    config_t(std::uint64_t size = 1 * 1024 * 1024, const std::string& marker = std::string()) :
        size(size),
        marker(marker)
    {}
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
