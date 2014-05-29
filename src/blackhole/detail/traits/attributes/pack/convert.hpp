#pragma once

#include <sstream>
#include <type_traits>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/supports/stream_push.hpp"
#include "blackhole/utils/lazy.hpp"

namespace blackhole {

namespace aux {

//! \brief Converts various types to log event attributes.
/*! Helper hierarchy of template classes that allows to pass objects of user
 *  defined classes as attribute in main logging macro. To make this possible,
 *  user defined class must have fully defined stream push `operator<<`.
 *  The logic is as follows: if the object can be implicitly converted to the
 *  `attribute_value_t` object, then that convertion is used.
 *  Otherwise the library would check via SFINAE if the custom class has defined
 *  stream push `operator<<` and, if yes - uses it.
 *  Otherwise static assert with human-readable message is triggered.
*/
template<typename T, class = void>
struct conv;

template<typename T>
struct conv<T, typename std::enable_if<
        log::attribute::is_constructible<T>::value>::type
    >
{
    static log::attribute_value_t from(T&& value) {
        return log::attribute_value_t(std::forward<T>(value));
    }
};

template<typename T>
struct conv<T, typename std::enable_if<
        !log::attribute::is_constructible<T>::value &&
        traits::supports::stream_push<T>::value>::type
    >
{
    static log::attribute_value_t from(T&& value) {
        std::ostringstream stream;
        stream << value;
        return log::attribute_value_t(stream.str());
    }
};

template<typename T>
struct conv<T, typename std::enable_if<
        !log::attribute::is_constructible<T>::value &&
        !traits::supports::stream_push<T>::value>::type
    >
{
    static log::attribute_value_t from(T&&) {
        static_assert(lazy_false<T>::value, "stream operator<< is not defined for type `T`");
        return log::attribute_value_t();
    }
};

} // namespace aux

} // namespace blackhole
