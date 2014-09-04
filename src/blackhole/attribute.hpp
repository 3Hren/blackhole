#pragma once

#include <ctime>
#include <initializer_list>
#include <utility>

#include "blackhole/attribute/name.hpp"
#include "blackhole/attribute/set.hpp"
#include "blackhole/attribute/view.hpp"
#include "blackhole/attribute/traits.hpp"
#include "blackhole/attribute/value.hpp"
#include "blackhole/detail/config/underlying.hpp"

namespace blackhole {

namespace attribute {

/// Simple typedef for simplifying attributes initialization.
typedef std::initializer_list<std::pair<name_t, value_t>> list;

/// Dynamic attribute factory function.
template<typename T>
inline
pair_t
make(const name_t& name, T&& value) {
    return std::make_pair(name, attribute_t(std::forward<T>(value)));
}

/// Attribute packing/extracting traits.
template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const set_view_t& attributes, const name_t& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

/// Attribute packing/extracting trait specialization for enumeration types.
template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename aux::underlying_type<T>::type underlying_type;

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const set_view_t& attributes, const name_t& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name).value));
    }
};

} // namespace attribute

} // namespace blackhole
