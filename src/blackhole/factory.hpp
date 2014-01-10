#pragma once

#include <vector>

#include <boost/any.hpp>

namespace blackhole {

template<typename T>
struct factory_traits {
    static typename T::config_type map_config(const boost::any& config);
};

namespace aux {

template<typename T>
static void any_to(const boost::any& from, T& to) {
    to = boost::any_cast<T>(from);
}

template<std::size_t N>
static void extract(const std::vector<boost::any>&) {}

template<std::size_t N, typename Head, typename... Other>
static void extract(const std::vector<boost::any>& vector, Head& current, Other&... other) {
    any_to(vector.at(N), current);
    extract<N + 1>(vector, other...);
}

template<typename... Args>
static void vector_to(const boost::any& from, Args&... args) {
    std::vector<boost::any> vector;
    any_to(from, vector);
    extract<0>(vector, args...);
}

} // namespace aux

} // namespace blackhole
