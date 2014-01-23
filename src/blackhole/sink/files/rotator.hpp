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
};

}

class null_rotator_t {
public:
    static const char* name() {
        return "";
    }
};

class rotator_t {
public:
    static const char* name() {
        return "/rotate";
    }
};

}

}
