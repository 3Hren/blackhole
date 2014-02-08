#pragma once

#include <cstdint>
#include <string>

#include <boost/assert.hpp>

#include "integral.hpp"

namespace blackhole {

namespace repository {

namespace config {

namespace conversion {

namespace aux {

template<class Builder, typename IntegralType>
inline void convert(Builder& builder, const std::string& name, const std::string& path, IntegralType value) {
    using namespace conversion;

    auto it = convertion.find(path + "/" + name);
    if (it == convertion.end()) {
        builder[name] = static_cast<int>(value);
        return;
    }

    const integral ic = it->second;
    switch (ic) {
    case integral::uint16:
        builder[name] = static_cast<std::uint16_t>(value);
        break;
    case integral::uint32:
        builder[name] = static_cast<std::uint32_t>(value);
        break;
    case integral::uint64:
        builder[name] = static_cast<std::uint64_t>(value);
        break;
    case integral::int16:
        builder[name] = static_cast<std::int16_t>(value);
        break;
    case integral::int32:
        builder[name] = static_cast<std::int32_t>(value);
        break;
    case integral::int64:
        builder[name] = static_cast<std::int64_t>(value);
        break;
    default:
        BOOST_ASSERT(false);
    }
}

} // namespace aux

} // namespace conversion

} // namespace config

} // namespace repository

} // namespace blackhole
