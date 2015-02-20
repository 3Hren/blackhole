#pragma once

#include "blackhole/config.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/supports/stream_push.hpp"

BLACKHOLE_BEG_NS

template<typename T>
struct is_convertible : public std::conditional<
        attribute::is_constructible<T>::value || traits::supports::stream_push<T>::value,
        std::true_type,
        std::false_type
    >
{};

BLACKHOLE_END_NS
