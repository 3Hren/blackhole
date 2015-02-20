#pragma once

#include <memory>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace aux {

namespace util {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace util

} // namespace aux

BLACKHOLE_END_NS
