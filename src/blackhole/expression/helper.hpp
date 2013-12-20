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
    T extracter;
    typename T::result_type other;

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) == other;
    }
};

template<typename T>
struct Less {
    T extracter;
    typename T::result_type other;

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) < other;
    }
};

template<typename T>
struct LessEq {
    T extracter;
    typename T::result_type other;

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) <= other;
    }
};

template<typename T>
struct Gt {
    T extracter;
    typename T::result_type other;

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) > other;
    }
};

} // namespace aux

} // namespace expression

} // namespace blackhole
