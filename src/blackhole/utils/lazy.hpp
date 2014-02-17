#pragma once

namespace blackhole {

template<typename T>
struct lazy_false {
    static const bool value = sizeof(T) == -1;
};

} // namespace blackhole
