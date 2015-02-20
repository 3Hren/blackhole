#pragma once

#include "blackhole/config.hpp"

#include "blackhole/detail/util/lazy.hpp"
#include "blackhole/dynamic.hpp"
#include "blackhole/error.hpp"
#include "blackhole/repository/factory/traits/config.hpp"
#include "blackhole/repository/factory/traits/extract.hpp"

BLACKHOLE_BEG_NS

template<class T>
struct factory_traits {
    static_assert(
        aux::util::lazy_false<T>::value,
        "You should implement 'factory_traits' template specialization for "
        "your type to properly map generic config object on real config."
    );
};

namespace aux {

template<class T>
struct config_mapper {
    typedef typename T::config_type config_type;

    static config_type map(const dynamic_t& input) {
        const extractor<T> ex(input);
        config_type config;
        factory_traits<T>::map_config(ex, config);
        return config;
    }
};

} // namespace aux

BLACKHOLE_END_NS
