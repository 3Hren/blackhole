#pragma once

#include <memory>

namespace blackhole {

namespace aux {

namespace util {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace util

} // namespace aux

} // namespace blackhole
