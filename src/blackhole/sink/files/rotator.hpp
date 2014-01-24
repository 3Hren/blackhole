#pragma once

#include <cstdint>
#include <string>

namespace blackhole {

namespace sink {

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

class rotator_t {
    rotator::config_t config;
public:
    static const char* name() {
        return "/rotate";
    }

    rotator_t(const rotator::config_t& config = rotator::config_t()) :
        config(config)
    {}

    template<typename Backend>
    bool necessary(Backend&) const {
        return false;
    }

    template<typename Backend>
    void rotate(Backend&) const {
        //!@todo: Implement me.
    }
};

}

}
