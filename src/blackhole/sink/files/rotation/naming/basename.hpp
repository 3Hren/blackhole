#pragma once

#include <string>

#include <boost/algorithm/string.hpp>

namespace blackhole {

namespace sink {

namespace rotation {

namespace naming {

class basename_t {
    const std::string pattern;

public:
    basename_t(const std::string& pattern = std::string()) :
        pattern(pattern)
    {}

    std::string transform(const std::string& filename) const {
        if (pattern.find("%(filename)s") != std::string::npos) {
            return boost::algorithm::replace_all_copy(pattern, "%(filename)s", filename);
        }

        return pattern;
    }
};

} // namespace naming

} // namespace rotation

} // namespace sink

} // namespace blackhole
