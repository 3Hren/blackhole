#pragma once

#include <string>

#include "blackhole/sink/files/rotation.hpp"

namespace blackhole {

namespace sink {

namespace files {

template<class Rotator = NoRotation>
struct config_t {
    std::string path;
    bool autoflush;

    config_t(const std::string& path = "/dev/stdout", bool autoflush = true) :
        path(path),
        autoflush(autoflush)
    {}
};

template<class Backend, class Watcher, class Timer>
struct config_t<rotator_t<Backend, Watcher, Timer>> {
    std::string path;
    bool autoflush;
    rotation::config_t<Watcher> rotation;

    config_t(const std::string& path = "/dev/stdout",
             bool autoflush = true,
             const rotation::config_t<Watcher>& rotation = rotation::config_t<Watcher>()) :
        path(path),
        autoflush(autoflush),
        rotation(rotation)
    {}
};

} // namespace files

} // namespace sink

} // namespace blackhole
