#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

namespace blackhole {

namespace aux {

namespace action {

struct empty {
    template<typename T>
    bool operator()(const T& value) const {
        return value.empty();
    }
};

} // namespace action

inline
std::vector<std::string>
split(const std::string& input, const std::string& separator) {
    std::vector<std::string> result;
    boost::split(result, input, boost::is_any_of(separator));
    result.erase(
        std::remove_if(result.begin(), result.end(), action::empty()),
        result.end()
    );
    return result;
}

} // namespace aux

} // namespace blackhole
