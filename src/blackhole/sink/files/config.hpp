#pragma once

#include <string>

#include "blackhole/config.hpp"

#include "blackhole/sink/files/rotation.hpp"

BLACKHOLE_BEG_NS

namespace sink {

namespace files {

template<class Rotator = null_rotator_t>
struct config_t {
    std::string path;
    bool autoflush;

    config_t(const std::string& path = "/dev/stdout", bool autoflush = true) :
        path(path),
        autoflush(autoflush)
    {}
};

template<class Backend, class Watcher, class Timer>
struct config_t<rotator_t<Backend, Watcher, Timer>> : public config_t<> {
    rotation::config_t<Watcher> rotation;

    config_t(const std::string& path = "/dev/stdout",
             bool autoflush = true,
             const rotation::config_t<Watcher>& rotation = rotation::config_t<Watcher>()) :
        config_t<>(path, autoflush),
        rotation(rotation)
    {}
};

} // namespace files

} // namespace sink

BLACKHOLE_END_NS
