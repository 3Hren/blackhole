#pragma once

#include "blackhole/attribute.hpp"
#include "blackhole/filter.hpp"

namespace blackhole {

namespace expression {

namespace aux {

struct And {
    filter_t first;
    filter_t second;

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) && second(attributes);
    }
};

template<typename T>
struct Eq {
    T first;
    typename T::result_type second;

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) == second;
    }
};

} // namespace aux

} // namespace expression

} // namespace blackhole
