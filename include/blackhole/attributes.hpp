#pragma once

#include <functional>
#include <string>

#ifdef __has_include
    #if __has_include(<boost/container/small_vector.hpp>)
        #define BLACKHOLE_HAVE_SMALL_VECTOR 1
    #else
        #define BLACKHOLE_HAVE_SMALL_VECTOR 0
    #endif
#endif

#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
    #include <boost/container/small_vector.hpp>
#else
    #include <vector>
#endif

#include "blackhole/attribute.hpp"

namespace blackhole {

// TODO: Try to encapsulate.
#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
typedef boost::container::small_vector<std::pair<string_view, attribute::value_t>, 16> attributes_t;
typedef boost::container::small_vector<std::pair<std::string, attribute::owned_t>, 16> attributes_w_t;
typedef boost::container::small_vector<std::reference_wrapper<const attributes_t>, 16> range_t;
#else
typedef std::vector<std::pair<string_view, attribute::value_t>> attributes_t;
typedef std::vector<std::pair<std::string, attribute::owned_t>> attributes_w_t;
// TODO: I can use stack allocator with fallback for portability.
typedef std::vector<std::reference_wrapper<const attributes_t>> range_t;
#endif

}  // namespace blackhole
