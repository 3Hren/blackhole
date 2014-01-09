#pragma once

namespace blackhole {

namespace action {

struct empty {
    template<typename T>
    bool operator ()(const T& value) const {
        return value.empty();
    }
};

} // namespace action

} // namespace blackhole
