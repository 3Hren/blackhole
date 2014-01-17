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

struct Or {
    filter_t first;
    filter_t second;

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) || second(attributes);
    }
};

template<typename T>
struct AndMixin {
    filter_t operator &&(filter_t other) const {
        return And { static_cast<const T&>(*this), other };
    }
};

template<typename T>
struct OrMixin {
    filter_t operator ||(filter_t other) const {
        return Or { static_cast<const T&>(*this), other };
    }
};

template<typename T>
struct Eq : public AndMixin<Eq<T>>, public OrMixin<Eq<T>> {
    T extracter;
    typename T::result_type other;

    Eq(T extracter, typename T::result_type other) : extracter(extracter), other(other) {}

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

template<typename T>
struct GtEq {
    T extracter;
    typename T::result_type other;

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) >= other;
    }
};

} // namespace aux

} // namespace expression

} // namespace blackhole
