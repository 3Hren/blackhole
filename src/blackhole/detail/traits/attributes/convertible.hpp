#pragma once

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/supports/stream_push.hpp"

namespace blackhole {

template<typename T>
struct is_convertible : public std::conditional<
        log::attribute::is_constructible<T>::value || traits::supports::stream_push<T>::value,
        std::true_type,
        std::false_type
    >
{};

} // namespace blackhole
