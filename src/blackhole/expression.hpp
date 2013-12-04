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

template<typename T>
struct has_attr_action_t {
    bool operator()(const log::attributes_t& attributes) const {
        return attributes.find(T::name()) != attributes.end();
    }

    filter_t operator &&(filter_t other) const {
        return And { *this, other };
    }
};

template<typename T>
has_attr_action_t<T> has_attr(const T&) {
    return has_attr_action_t<T>();
}

} // namespace expr

} // namespace blackhole
