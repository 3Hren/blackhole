#pragma once

#include <map>
#include <mutex>
#include <string>

#include <boost/optional.hpp>

namespace blackhole {

namespace repository {

namespace config {

namespace conversion {

namespace aux {

enum class integral_t {
    uint16,
    uint32,
    uint64,
    int16,
    int32,
    int64
};

} // namespace aux

} // namespace conversion

} // namespace config

} // namespace repository

} // namespace blackhole
