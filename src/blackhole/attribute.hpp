#pragma once

#include <ctime>
#include <initializer_list>
#include <utility>

#include "blackhole/attribute/name.hpp"
#include "blackhole/attribute/scope.hpp"
#include "blackhole/attribute/set.hpp"
#include "blackhole/attribute/view.hpp"
#include "blackhole/attribute/traits.hpp"
#include "blackhole/attribute/value.hpp"
#include "blackhole/utils/noexcept.hpp"
#include "blackhole/utils/timeval.hpp"
#include "blackhole/utils/types.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace attribute {

/*!
 * Simple typedef for attributes initializer list.
 * Useful when specifying local attributes.
 */
typedef std::initializer_list<std::pair<name_t, value_t>> list;

/*!
 * Dynamic attribute factory function.
 */
template<typename T>
inline
pair_t
make(const name_t& name, const T& value, scope_t scope = DEFAULT_SCOPE) {
    return std::make_pair(name, attribute_t(value, scope));
}

template<typename T>
inline
pair_t
make(const name_t& name, T&& value, scope_t scope = DEFAULT_SCOPE) {
    return std::make_pair(name, attribute_t(std::move(value), scope));
}

// Attribute packing/unpacking/extracting.
template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const set_view_t& attributes, const name_t& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

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
