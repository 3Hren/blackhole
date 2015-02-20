#pragma once

#include <type_traits>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

template<class, class>
struct or_ : public std::true_type {};

template<>
struct or_<std::false_type, std::false_type> : public std::false_type {};

BLACKHOLE_END_NS
