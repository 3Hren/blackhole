#pragma once

#include <memory>

namespace blackhole {
inline namespace v1 {

template<typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T> {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace v1
}  // namespace blackhole
