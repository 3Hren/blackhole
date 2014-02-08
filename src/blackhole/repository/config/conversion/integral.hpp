#pragma once

#include <map>
#include <string>

namespace blackhole {

namespace repository {

namespace config {

namespace conversion {

namespace aux {

enum class integral {
    uint16,
    uint32,
    uint64,
    int16,
    int32,
    int64
};

} // namespace aux

static std::map<std::string, aux::integral> convertion = {
    { "sink/files/rotation/backups", aux::integral::uint16 },
    { "sink/files/rotation/size", aux::integral::uint64 }
};

} // namespace conversion

} // namespace config

} // namespace repository

} // namespace blackhole
