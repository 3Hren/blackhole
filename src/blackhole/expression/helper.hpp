#pragma once

#include "blackhole/attribute.hpp"
#include "blackhole/filter.hpp"

namespace blackhole {

namespace expression {

namespace aux {

struct And;
struct Or;

template<typename T>
struct AndMixin {
    And operator &&(filter_t other) const;
};

template<typename T>
struct OrMixin {
    Or operator ||(filter_t other) const;
};

template<typename T>
struct LogicMixin : public AndMixin<T>, public OrMixin<T> {};

struct And : public LogicMixin<And> {
    filter_t first;
    filter_t second;

    And(filter_t first, filter_t second) : first(first), second(second) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) && second(attributes);
    }
};

struct Or : public LogicMixin<Or> {
    filter_t first;
    filter_t second;

    Or(filter_t first, filter_t second) : first(first), second(second) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return first(attributes) || second(attributes);
    }
};

template<typename T>
And AndMixin<T>::operator &&(filter_t other) const  {
    return And { static_cast<const T&>(*this), other };
}

template<typename T>
Or OrMixin<T>::operator ||(filter_t other) const {
    return Or { static_cast<const T&>(*this), other };
}

template<typename T>
struct Eq : public LogicMixin<Eq<T>> {
    T extracter;
    typename T::result_type other;

    Eq(T extracter, typename T::result_type other) : extracter(extracter), other(other) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) == other;
    }
};

template<typename T>
struct Less : public LogicMixin<Less<T>> {
    T extracter;
    typename T::result_type other;

    Less(T extracter, typename T::result_type other) : extracter(extracter), other(other) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) < other;
    }
};

template<typename T>
struct LessEq : public LogicMixin<LessEq<T>> {
    T extracter;
    typename T::result_type other;

    LessEq(T extracter, typename T::result_type other) : extracter(extracter), other(other) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) <= other;
    }
};

template<typename T>
struct Gt : public LogicMixin<Gt<T>> {
    T extracter;
    typename T::result_type other;

    Gt(T extracter, typename T::result_type other) : extracter(extracter), other(other) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) > other;
    }
};

template<typename T>
struct GtEq : public LogicMixin<GtEq<T>> {
    T extracter;
    typename T::result_type other;

    GtEq(T extracter, typename T::result_type other) : extracter(extracter), other(other) {}

    bool operator ()(const log::attributes_t& attributes) const {
        return extracter(attributes) >= other;
    }
};

} // namespace aux

} // namespace expression

} // namespace blackhole
