#pragma once

namespace blackhole {

namespace sink {

struct rotator_config_t {
    std::uint64_t size;
    std::uint16_t count;
};

struct null_rotator_t {
    static const char* name() {
        return "";
    }
};

struct rotator_t {
    static const char* name() {
        return "/rotate";
    }
};

}

}
