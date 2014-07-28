#pragma once

#include <map>
#include <string>

#include <boost/any.hpp>

#include "blackhole/dynamic.hpp"
#include "blackhole/error.hpp"

namespace blackhole {

namespace aux {

template<class T>
class extractor {
    dynamic_t source;
    std::string name;

public:
    extractor(dynamic_t source, std::string name = T::name()) :
        source(std::move(source)),
        name(std::move(name))
    {}

    extractor<T> operator[](const std::string& name) const {
        dynamic_t::object_t map;
        try {
            to(map);
        } catch (const std::exception&) {
            //!@todo: Throw specialized exceptions.
            throw error_t("can't extract '%s': '%s' is not map", name, this->name);
        }

        auto it = map.find(name);
        if (it == map.end()) {
            throw error_t("can't extract '%s' field from '%s': member is absent", name, T::name());
        }

        return extractor<T>(it->second, name);
    }

    template<typename R>
    void to(R& value) const {
        try {
            value = source.to<R>();
        } catch (const dynamic::bad_cast& err) {
            throw error_t("unable to extract '%s' field from '%s': %s", name, T::name(), err.what());
        }
    }

    template<typename R>
    R get() const {
        R value;
        to(value);
        return value;
    }

private:
};

} // namespace aux

}  // namespace blackhole
