#pragma once

#include <boost/any.hpp>

#include <blackhole/formatter/map/value.hpp>

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

} // namespace aux

namespace formatter {

class base_t {
protected:
    mapping::value_t mapper;

public:
    void set_mapper(const mapping::value_t& mapper) {
        this->mapper = mapper;
    }

    void set_mapper(mapping::value_t&& mapper) {
        this->mapper = std::move(mapper);
    }
};

} // namespace formatter

} // namespace blackhole
