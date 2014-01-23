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

    config_t() :
        size(10 * 1024 * 1024),
        count(5),
        suffix(".%N")
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
    void rotate(Backend&) const {}
};

}

}
