#pragma once

#include <map>
#include <vector>

#include <boost/any.hpp>

#include "blackhole/error.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

namespace aux {

template<typename T>
static bool is(const boost::any& any) {
    try {
        boost::any_cast<T>(any);
    } catch (const boost::bad_any_cast&) {
        return false;
    }

    return true;
}

template<typename T>
static void any_to(const boost::any& from, T& to) {
    to = boost::any_cast<T>(from);
}

namespace vector {

template<std::size_t N>
static void extract(const std::vector<boost::any>&) {}

template<std::size_t N, typename Head, typename... Other>
static void extract(const std::vector<boost::any>& vector, Head& current, Other&... other) {
    any_to(vector.at(N), current);
    extract<N + 1>(vector, other...);
}

} // namespace vector

namespace map {


} // namespace map

template<typename... Args>
static void vector_to(const boost::any& from, Args&... args) {
    std::vector<boost::any> vector;
    any_to(from, vector);
    vector::extract<0>(vector, args...);
}

template<typename Sink>
struct extractor {
    boost::any source;
    std::string name;

    extractor(const boost::any& source, const std::string& name = Sink::name()) :
        source(source),
        name(name)
    {}

    extractor<Sink> operator [](const std::string& name) const {
        std::map<std::string, boost::any> map;
        try {
            any_to(source, map);
        } catch (boost::bad_any_cast&) {
            throw error_t("can not extract '%s': '%s' is not map", name, this->name);
        }
        return extractor<Sink>({ map[name], name });
    }

    template<typename T>
    void to(T& value) {
        try {
            any_to(source, value);
        } catch (boost::bad_any_cast&) {
            throw error_t("conversion error for member '%s'", name);
        }
    }
};

} // namespace aux

} // namespace blackhole
