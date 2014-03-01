#pragma once

#include <type_traits>

namespace blackhole {

template<class T>
struct is_true_type {
    static const bool value = std::is_same<T, std::true_type>::value;
};

} // namespace blackhole
