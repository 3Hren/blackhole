#pragma once

#include <ostream>
#include <sstream>
#include <string>

namespace elasticsearch {

namespace utils {

namespace aux {

inline void substitute(std::ostringstream&) {}

template<typename... Args>
inline void substitute(std::ostringstream& stream,
                       const std::string& arg,
                       const Args&... args) {
    if (!arg.empty()) {
        stream << "/";

        if (arg != "/") {
            stream << arg;
        }
    }

    substitute(stream, args...);
}

} // namespace aux

template<typename... Args>
inline std::string make_path(const Args&... args) {
    std::ostringstream stream;
    aux::substitute(stream, args...);
    return stream.str();
}

} // namespace utils

} // namespace elasticsearch
