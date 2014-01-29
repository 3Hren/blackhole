#pragma once

#include <cstdint>
#include <string>

namespace blackhole {

namespace sink {

namespace rotation {

struct config_t {
    std::string pattern;
    std::uint16_t backups;
    std::uint64_t size;

    config_t(const std::string& pattern = ".%N", std::uint16_t backups = 5, std::uint64_t size = 10 * 1024 * 1024) :
        pattern(pattern),
        backups(backups),
        size(size)
    {}
};

} // namespace rotation

} // namespace sink

} // namespace blackhole
