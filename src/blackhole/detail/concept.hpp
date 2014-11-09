#pragma once

#include <type_traits>

#include <blackhole/attribute/set.hpp>
#include <blackhole/attribute/value.hpp>
#include <blackhole/forwards.hpp>
#include <blackhole/detail/traits/supports/stream_push.hpp>

// TODO: Deprecated!
namespace blackhole {

namespace concept {

#define BLACKHOLE_CONCEPT_CHECK(__name__, __result__, __signature__) \
    template<typename T> \
    struct can_##__name__ { \
    private: \
        template<typename U> \
        static decltype(support::declval<U>().__signature__) detect(const U&); \
        template<typename> \
        static std::false_type detect(...); \
    public: \
        typedef std::integral_constant< \
            bool, \
            std::is_same< \
                decltype(detect<T>(support::declval<T>())), \
                __result__ \
            >::value \
        > type; \
    }

BLACKHOLE_CONCEPT_CHECK(
    open_record,
    record_t,
    open_record()
);

BLACKHOLE_CONCEPT_CHECK(
    open_record_with_attribute_pair,
    record_t,
    open_record(support::declval<attribute::pair_t>())
);

BLACKHOLE_CONCEPT_CHECK(
    open_record_with_attribute_set,
    record_t,
    open_record(support::declval<attribute::set_t>())
);

BLACKHOLE_CONCEPT_CHECK(
    push_record,
    void,
    push(support::declval<record_t&&>())
);

template<class T>
struct logger :
    public std::integral_constant<
        bool,
        can_open_record<T>::type::value &&
        can_open_record_with_attribute_pair<T>::type::value &&
        can_open_record_with_attribute_set<T>::type::value &&
        can_push_record<T>::type::value
    >
{};

} // namespace concept

} // namespace blackhole
