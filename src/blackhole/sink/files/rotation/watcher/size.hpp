#pragma once

#include <cstdint>
#include <string>

#include "blackhole/sink/files/rotation/watcher/config.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

struct size_t {
    std::uint64_t size;

    size_t(std::uint64_t size) :
        size(size)
    {}

    size_t(const config_t<size_t>& config) :
        size(config.size)
    {}

    template<typename Backend>
    bool operator ()(Backend& backend, const std::string& message) const {
        return backend.size(backend.filename()) + message.size() >= size;
    }
};

} // namespace rotation

} // namespace watcher

} // namespace sink

} // namespace blackhole
