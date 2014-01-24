#pragma once

#include <cstdint>
#include <string>

namespace blackhole {

namespace sink {

//! Tag for file sinks with no rotation.
template<typename Backend> class NoRotation;

namespace rotator {

struct config_t {
    std::uint64_t size;
    std::uint16_t count;
    std::string suffix;

    config_t(std::uint64_t size = 10 * 1024 * 1024, std::uint16_t count = 5, const std::string& suffix = ".%N") :
        size(size),
        count(count),
        suffix(suffix)
    {}
};

}

template<typename Backend>
class rotator_t {
    rotator::config_t config;
    Backend& backend;
public:
    static const char* name() {
        return "/rotate";
    }

    rotator_t(Backend& backend) :
        backend(backend)
    {}

    rotator_t(const rotator::config_t& config, Backend& backend) :
        config(config),
        backend(backend)
    {}

    bool necessary() const {
        return false;
    }

    void rotate() const {
        //!@todo: Implement me.
    }
};

}

}
