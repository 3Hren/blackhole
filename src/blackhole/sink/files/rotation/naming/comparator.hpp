#pragma once

#include <string>

namespace blackhole {

namespace sink {

namespace comparator {

namespace time {

template<typename Backend>
struct ascending {
    Backend& backend;

    ascending(Backend& backend) : backend(backend) {}

    bool operator ()(const std::string& lhs, const std::string& rhs) const {
        return backend.changed(lhs) < backend.changed(rhs);
    }
};

} // namespace time

} // namespace comparator

} // namespace sink

} // namespace blackhole
