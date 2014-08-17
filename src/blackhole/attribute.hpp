#pragma once

#include <cstdint>
#include <ctime>
#include <initializer_list>
#include <unordered_map>
#include <iterator>
#include <stdexcept>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include "blackhole/attribute/name.hpp"
#include "blackhole/attribute/scope.hpp"
#include "blackhole/attribute/set.hpp"
#include "blackhole/attribute/view.hpp"
#include "blackhole/attribute/traits.hpp"
#include "blackhole/attribute/value.hpp"

#include "blackhole/platform/initializer_list.hpp"
#include "utils/timeval.hpp"
#include "utils/types.hpp"
#include "utils/underlying.hpp"
#include "blackhole/utils/noexcept.hpp"

namespace blackhole {

namespace attribute {

// Simple typedef for attributes initializer list. Useful when specifying local attributes.
typedef std::initializer_list<std::pair<std::string, attribute::value_t>> list;

// Dynamic attribute factory function.
template<typename T>
inline attribute::pair_t make(const std::string& name, const T& value, attribute::scope_t scope = attribute::DEFAULT_SCOPE) {
    return std::make_pair(name, attribute_t(value, scope));
}

template<typename T>
inline attribute::pair_t make(const std::string& name, T&& value, attribute::scope_t scope = attribute::DEFAULT_SCOPE) {
    return std::make_pair(name, attribute_t(std::move(value), scope));
}

// Attribute packing/unpacking/extracting.
template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const attribute::set_view_t& attributes, const std::string& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename aux::underlying_type<T>::type underlying_type;

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const attribute::set_view_t& attributes, const std::string& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name).value));
    }
};

} // namespace attribute

} // namespace blackhole
