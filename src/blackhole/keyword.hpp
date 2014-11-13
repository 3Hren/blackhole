#pragma once

#include <boost/preprocessor/cat.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/config.hpp"
#include "blackhole/filter.hpp"

#define BLACKHOLE_AUX_KEYWORD_TAG_TYPE(name) BOOST_PP_CAT(name, _t)
#define BLACKHOLE_AUX_KEYWORD_TYPE(name, T) \
    ::blackhole::keyword::keyword_t<T, tag::BLACKHOLE_AUX_KEYWORD_TAG_TYPE(name)>

#define BLACKHOLE_AUX_DECLARE_KEYWORD(_name_, T) \
    namespace keyword {                                                         \
    namespace tag {                                                             \
        struct BLACKHOLE_AUX_KEYWORD_TAG_TYPE(_name_) {                         \
            typedef T type;                                                     \
            static const char* name() { return BOOST_PP_STRINGIZE(_name_); }    \
        };                                                                      \
    }                                                                           \
    static inline BLACKHOLE_AUX_KEYWORD_TYPE(_name_, T)& _name_() {             \
        static BLACKHOLE_AUX_KEYWORD_TYPE(_name_, T) self;                      \
        return self;                                                            \
    }                                                                           \
    }

#define DECLARE_LOCAL_KEYWORD(Name, T)    BLACKHOLE_AUX_DECLARE_KEYWORD(Name, T)
#define DECLARE_EVENT_KEYWORD(Name, T)    BLACKHOLE_AUX_DECLARE_KEYWORD(Name, T)
#define DECLARE_KEYWORD(Name, T)          BLACKHOLE_AUX_DECLARE_KEYWORD(Name, T)
#define DECLARE_THREAD_KEYWORD(Name, T)   BLACKHOLE_AUX_DECLARE_KEYWORD(Name, T)
#define DECLARE_UNIVERSE_KEYWORD(Name, T) BLACKHOLE_AUX_DECLARE_KEYWORD(Name, T)

#include "blackhole/expression/helper.hpp"

namespace blackhole {

namespace keyword {

template<typename T, typename NameProvider>
struct keyword_t {
    typedef T type;
    static_assert(attribute::is_supported<
        typename blackhole::aux::underlying_type<T>::type
    >::value, "invalid attribute type");

    static const char* name() {
        return NameProvider::name();
    }

    struct extracter_t {
        typedef typename blackhole::aux::underlying_type<T>::type result_type;

        result_type operator()(const attribute::combined_view_t& attributes) const {
            return static_cast<result_type>(attribute::traits<T>::extract(attributes, name()));
        }
    };

    attribute::pair_t operator=(const T& value) const {
        return attribute::make(name(), attribute::traits<T>::pack(value));
    }

    attribute::pair_t operator=(T&& value) const {
        return attribute::make(name(), attribute::traits<T>::pack(std::forward<T>(value)));
    }

    template<template<typename> class Operator>
    inline
    Operator<extracter_t> operation(const T& other) const {
        typedef typename blackhole::aux::underlying_type<T>::type result_type;
        return Operator<extracter_t>({ extracter_t(), static_cast<result_type>(other) });
    }

    typename expression::aux::Less<extracter_t> operator<(const T& other) const {
        return operation<expression::aux::Less>(other);
    }

    typename expression::aux::LessEq<extracter_t> operator<=(const T& other) const {
        return operation<expression::aux::LessEq>(other);
    }

    typename expression::aux::Gt<extracter_t> operator>(const T& other) const {
        return operation<expression::aux::Gt>(other);
    }

    typename expression::aux::GtEq<extracter_t> operator>=(const T& other) const {
        return operation<expression::aux::GtEq>(other);
    }

    typename expression::aux::Eq<extracter_t> operator==(const T& other) const {
        return operation<expression::aux::Eq>(other);
    }
};

} // namespace keyword

} // namespace blackhole
