#pragma once

#include <string>
#include <sstream>

#include <boost/format.hpp>

namespace blackhole {

namespace utils {

namespace aux {

static inline
std::string
substitute(boost::format&& message) {
    return message.str();
}

template<typename T, typename... Args>
static inline
std::string
substitute(boost::format&& message, const T& argument, const Args&... args) {
    return substitute(std::move(message % argument), args...);
}

} // namespace aux

template<typename... Args>
static inline
std::string
format(const std::string& format, const Args&... args) {
    try {
        return aux::substitute(boost::format(format), args...);
    } catch(const boost::io::format_error& e) {
        std::ostringstream stream;
        stream << "<unable to format the message - " << e.what() << ">";
        return stream.str();
    }
}

} } // namespace blackhole::utils
