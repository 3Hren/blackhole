#pragma once

#include <cstdint>
#include <string>

#include "blackhole/sink/files/rotation/watcher/config.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

template<typename Watcher>
struct config_t {
    std::string pattern;
    std::uint16_t backups;
    watcher::config_t<Watcher> watcher;

    config_t(const std::string& pattern = "%(filename)s.%N",
             std::uint16_t backups = 5,
             watcher::config_t<Watcher> watcher = watcher::config_t<Watcher>()) :
        pattern(pattern),
        backups(backups),
        watcher(watcher)
    {}
};

template<>
struct config_t<watcher::move_t> {
    watcher::config_t<watcher::move_t> watcher;

    config_t() {}
};

} // namespace rotation

} // namespace sink

} // namespace blackhole
