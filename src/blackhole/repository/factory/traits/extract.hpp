#pragma once

#include <map>
#include <string>

#include <boost/any.hpp>

#include "blackhole/error.hpp"
#include "cast.hpp"

namespace blackhole {

namespace aux {

template<class T>
struct extractor {
    boost::any source;
    std::string name;

    extractor(const boost::any& source, const std::string& name = T::name()) :
        source(source),
        name(name)
    {}

    extractor<T> operator [](const std::string& name) const {
        std::map<std::string, boost::any> map;
        try {
            to(map);
        } catch (const std::exception&) {
            throw error_t("can not extract '%s': '%s' is not map", name, this->name);
        }
        return extractor<T>(map[name], name);
    }

    template<typename R>
    void to(R& value) const {
        try{
            cast_traits<R>::to(source, value);
        } catch (boost::bad_any_cast&) {
            throw error_t("conversion error for member '%s'", name);
        }
    }

    template<typename R>
    R get() const {
        R value;
        to(value);
        return value;
    }
};

} // namespace aux

}  // namespace blackhole
