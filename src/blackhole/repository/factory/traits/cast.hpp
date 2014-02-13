#pragma once

#include <boost/any.hpp>
#include <boost/mpl/for_each.hpp>

#include "integer.hpp"

namespace blackhole {

namespace aux {

template<typename T>
static void any_to(const boost::any& from, T& to) {
    to = boost::any_cast<T>(from);
}

template<typename T>
struct cast {
    const boost::any& source;
    T& value;
    bool& successful;

    cast(const boost::any& source, T& value, bool& successful) :
        source(source),
        value(value),
        successful(successful)
    {}

    template<typename Integer>
    void operator ()(Integer) {
        if (successful) {
            return;
        }

        const Integer* value_ptr = boost::any_cast<Integer>(&source);
        if (value_ptr) {
            value = *value_ptr;
            successful = true;
        }
    }
};

template<typename T, class = void> struct cast_traits;

template<typename T>
struct cast_traits<T, typename std::enable_if<!is_supported_integer<T>::value>::type> {
    static void to(const boost::any& source, T& value) {
        any_to(source, value);
    }
};

// When parsing external configs (like json) there are no information about
// the exact size of integer types.
// But for configs it is very unlikely to keep all integer variables under `int` without size
// specification, for example - port is always keeped as std::uint16_t.
// So, after parsing we have raw `int` type stored in boost::any.
// On the other side final config awaits concrete type. To connect them, I've decided to
// iterate over all supported integers in library (total 8) and try to convert boost::any to
// each of them in sequence until successful.
template<typename T>
struct cast_traits<T, typename std::enable_if<is_supported_integer<T>::value>::type> {
    static void to(const boost::any& source, T& value) {
        bool successful = false;
        boost::mpl::for_each<supported_integers_t>(cast<T>(source, value, successful));
        if (!successful) {
            throw blackhole::error_t("unsupported integer type");
        }
    }
};

} // namespace aux

} // namespace blackhole
