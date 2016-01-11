#pragma once

#include <functional>
#include <string>

#ifdef __has_include
    #if __has_include(<boost/container/small_vector.hpp>)
        #define BLACKHOLE_HAVE_SMALL_VECTOR
    #endif
#endif

#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
    // TODO: Use stack composable allocator instead.
    #include <boost/container/small_vector.hpp>
#else
    #include <vector>
#endif

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
inline namespace v1 {

template<typename T>
struct view_of;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace attribute {

class value_t;
class view_t;

}  // namespace attribute
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {

typedef std::pair<std::string, attribute::value_t> attribute_t;

#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
typedef boost::container::small_vector<attribute_t, 16> attributes_t;
#else
typedef std::vector<attribute_t> attributes_t;
#endif

template<>
struct view_of<attribute_t> {
    typedef string_view key_type;
    typedef attribute::view_t value_type;

    typedef std::pair<key_type, value_type> type;
};

template<>
struct view_of<attributes_t> {
    typedef view_of<attribute_t>::type value_type;

#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
    typedef boost::container::small_vector<value_type, 16> type;
#else
    typedef std::vector<value_type> type;
#endif
};

/// Convenient typedef for attributes set view.
typedef view_of<attributes_t>::type attribute_list;

#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
typedef boost::container::small_vector<std::reference_wrapper<const view_of<attributes_t>::type>, 16> attribute_pack;
#else
// TODO: I can use stack allocator with fallback for portability.
typedef std::vector<std::reference_wrapper<const view_of<attributes_t>::type>> attribute_pack;
#endif

}  // namespace v1
}  // namespace blackhole
