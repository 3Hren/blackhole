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

    const std::string name;

    template<template<typename> class Operator>
    static inline
    Operator<get_attr_action_t<T> > operation(const get_attr_action_t<T>& extractor, const T& other) {
        return Operator<get_attr_action_t<T>>({ extractor, static_cast<result_type>(other) });
    }

    result_type operator ()(const log::attributes_t& attributes) const {
        // GCC 4.4. can't compare strongly-typed enums.
        return static_cast<result_type>(attribute::traits<T>::extract(attributes, name));
    }

    filter_t operator ==(const T& other) const {
        return operation<aux::Eq>(*this, other);
    }

    filter_t operator <(const T& other) const {
        return operation<aux::Less>(*this, other);
    }

    filter_t operator <=(const T& other) const {
        return operation<aux::LessEq>(*this, other);
    }

    filter_t operator >(const T& other) const {
        return operation<aux::Gt>(*this, other);
    }

    filter_t operator >=(const T& other) const {
        return operation<aux::GtEq>(*this, other);
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
