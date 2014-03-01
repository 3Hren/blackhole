#pragma once

#include <type_traits>

namespace blackhole {

template<class, class>
struct or_ : public std::true_type {};

template<>
struct or_<std::false_type, std::false_type> : public std::false_type {};

} // namespace blackhole
