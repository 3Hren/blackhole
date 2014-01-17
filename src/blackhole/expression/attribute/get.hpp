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
    typedef typename blackhole::aux::underlying_type<T>::type result_type;

    template<template<typename = T> class Operator>
    struct filtered {
        typedef Operator<get_attr_action_t<T>> type;
    };

    const std::string name;

    template<template<typename = T> class Operator>
    inline
    Operator<get_attr_action_t<T> > operation(const T& other) const {
        return Operator<get_attr_action_t<T>>({ *this, static_cast<result_type>(other) });
    }

    result_type operator ()(const log::attributes_t& attributes) const {
        // GCC 4.4. can't compare strongly-typed enums.
        return static_cast<result_type>(attribute::traits<T>::extract(attributes, name));
    }

    typename filtered<aux::Eq>::type operator ==(const T& other) const {
        return operation<aux::Eq>(other);
    }

    typename filtered<aux::Less>::type operator <(const T& other) const {
        return operation<aux::Less>(other);
    }

    typename filtered<aux::LessEq>::type operator <=(const T& other) const {
        return operation<aux::LessEq>(other);
    }

    typename filtered<aux::Gt>::type operator >(const T& other) const {
        return operation<aux::Gt>(other);
    }

    typename filtered<aux::GtEq>::type operator >=(const T& other) const {
        return operation<aux::GtEq>(other);
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
