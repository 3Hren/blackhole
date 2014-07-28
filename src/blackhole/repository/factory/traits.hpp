#pragma once

#include <map>

#include <boost/any.hpp>

#include "traits/config.hpp"
#include "traits/extract.hpp"
#include "traits/integer.hpp"
#include "traits/unique.hpp"
#include "blackhole/dynamic.hpp"
#include "blackhole/error.hpp"
#include "blackhole/utils/lazy.hpp"

namespace blackhole {

template<class T>
struct factory_traits {
    static_assert(
        lazy_false<T>::value,
        "You should implement 'factory_traits' template specialization for "
        "your type to properly map generic config object on real config."
    );
};

namespace aux {

template<class T>
struct config_mapper {
    typedef typename T::config_type config_type;

    static config_type map(const dynamic_t& input) {
        const aux::extractor<T> extractor(input);
        config_type config;
        factory_traits<T>::map_config(extractor, config);
        return config;
    }
};

} // namespace aux

} // namespace blackhole
