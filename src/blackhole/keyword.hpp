#pragma once

#include "attribute.hpp"
#include "filter.hpp"
#include "utils/actions/keyword.hpp"

#define DECLARE_KEYWORD_IMPL(Name, Scope, T) \
    namespace tag { \
        static_assert(blackhole::log::attribute::is_supported<T>::value, "invalid attribute type"); \
        struct Name##_t { \
            typedef T type; \
            static const char* name() { return #Name; } \
        }; \
    } \
    static inline blackhole::keyword::keyword_t<T, tag::Name##_t, log::attribute::scope::Scope>& Name() { \
        static blackhole::keyword::keyword_t<T, tag::Name##_t, log::attribute::scope::Scope> self; \
        return self; \
    }

#define DECLARE_EVENT_KEYWORD(Name, T) \
    DECLARE_KEYWORD_IMPL(Name, event, T)

#define DECLARE_KEYWORD(Name, T) \
    DECLARE_KEYWORD_IMPL(Name, global, T)

#define DECLARE_THREAD_KEYWORD(Name, T) \
    DECLARE_KEYWORD_IMPL(Name, thread, T)

#define DECLARE_UNIVERSE_KEYWORD(Name, T) \
    DECLARE_KEYWORD_IMPL(Name, universe, T)

#include "blackhole/expression/helper.hpp"

namespace blackhole {

namespace keyword {

template<typename T, typename NameProvider, log::attribute::scope Scope = log::attribute::DEFAULT_SCOPE>
struct keyword_t {
    typedef T type;

    static const char* name() {
        return NameProvider::name();
    }

    struct extracter_t {
        typedef typename blackhole::aux::underlying_type<T>::type result_type;

        result_type operator()(const log::attributes_t& attributes) const {
            return static_cast<result_type>(attribute::traits<T>::extract(attributes, name()));
        }
    };

    log::attribute_pair_t operator=(const T& value) const {
        return attribute::make(name(), attribute::traits<T>::pack(value), Scope);
    }

    log::attribute_pair_t operator=(T&& value) const {
        return attribute::make(name(), attribute::traits<T>::pack(std::forward<T>(value)), Scope);
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

    typename expression::aux::GtEq<extracter_t> operator>=(const T& other) const {
        return operation<expression::aux::GtEq>(other);
    }

    typename expression::aux::Eq<extracter_t> operator==(const T& other) const {
        return operation<expression::aux::Eq>(other);
    }
};

} // namespace keyword

} // namespace blackhole
