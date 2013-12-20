#pragma once

#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/expression/helper.hpp"
#include "blackhole/filter.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace expression {

template<typename T>
struct get_attr_action_t {
    typedef T result_type;

    const std::string name;

    result_type operator ()(const log::attributes_t& attributes) const {
        return attribute::traits<T>::extract(attributes, name);
    }

    filter_t operator ==(const T& other) const {
        return aux::Eq<get_attr_action_t<T>>({ *this, other });
    }

    filter_t operator <(const T& other) const {
        return aux::Less<get_attr_action_t<T>>({ *this, other });
    }

    filter_t operator <=(const T& other) const {
        return aux::LessEq<get_attr_action_t<T>>({ *this, other });
    }
};

template<typename T>
get_attr_action_t<T> get_attr(const std::string& name) {
    return get_attr_action_t<T>({ name });
}

template<typename T>
get_attr_action_t<typename T::type> get_attr(const T&) {
    return get_attr<typename T::type>(std::string(T::name()));
}

} // namespace expression

} // namespace blackhole
