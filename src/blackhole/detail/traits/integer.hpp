#pragma once

#include <type_traits>

namespace blackhole {

namespace type_traits {

template<typename T>
struct is_integer :
    public std::conditional<
        std::is_integral<T>::value && !std::is_same<T, bool>::value,
        std::true_type,
        std::false_type
    >::type
{};

template<typename T>
struct is_unsigned_integer :
    public std::conditional<
        is_integer<T>::value && std::is_unsigned<T>::value,
        std::true_type,
        std::false_type
    >::type
{};

template<typename T>
struct is_signed_integer :
    public std::conditional<
        is_integer<T>::value && std::is_signed<T>::value,
        std::true_type,
        std::false_type
    >::type
{};

} // namespace type_traits

} // namespace blackhole
