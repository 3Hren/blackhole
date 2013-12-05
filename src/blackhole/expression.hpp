#pragma once

#include "attribute.hpp"
#include "filter.hpp"

namespace blackhole {

namespace expr {

struct And {
    filter_t first;
    filter_t second;

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) && second(attributes);
    }
};

template<typename L, typename R>
struct Eq {
    L first;
    R second;

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) == second;
    }
};

template<typename T>
struct has_attr_action_t {
    const std::string name;

    bool operator()(const log::attributes_t& attributes) const {
        return attributes.find(name) != attributes.end();
    }

    filter_t operator &&(filter_t other) const {
        return And { *this, other };
    }
};

template<typename T>
has_attr_action_t<T> has_attr(const std::string& name) {
    return has_attr_action_t<T>({ name });
}

template<typename T>
has_attr_action_t<T> has_attr(const T&) {
    return has_attr<T>(std::string(T::name()));
}

template<typename T>
struct get_attr_action_t {
    const std::string name;

    const T& operator()(const log::attributes_t& attributes) const {
        return boost::get<T>(attributes.at(name));
    }

    filter_t operator ==(const T& other) const {
        return Eq<get_attr_action_t<T>, T>({ *this, other });
    }
};

template<typename T>
get_attr_action_t<T> get_attr(const std::string& name) {
    return get_attr_action_t<T>({ name });
}

} // namespace expr

} // namespace blackhole
