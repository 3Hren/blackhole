#pragma once

#include <cstdint>
#include <string>

#include <boost/mpl/set.hpp>

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

struct size_t;
struct datetime_t;

template<class Watcher>
struct config_t;

template<>
struct config_t<watcher::size_t> {
    std::uint64_t size;
};

template<>
struct config_t<watcher::datetime_t> {
    std::string marker;
};

template<>
struct config_t<boost::mpl::set<watcher::size_t, watcher::datetime_t>> {
    std::uint64_t size;
    std::string marker;
};

}

}

}

}
