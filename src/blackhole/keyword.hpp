#pragma once

#include "attribute.hpp"
#include "filter.hpp"
#include "helper.hpp"

#define DECLARE_KEYWORD_IMPL(Name, Scope, T) \
    namespace tag { \
        struct Name##_t { \
            static const char* name() { return #Name; } \
        }; \
    } \
    static blackhole::keyword::keyword_t<T, tag::Name##_t, log::attribute::scope::Scope>& Name() { \
        static blackhole::keyword::keyword_t<T, tag::Name##_t, log::attribute::scope::Scope> self; \
        return self; \
    }

#define DECLARE_EVENT_KEYWORD(Name, T) \
    DECLARE_KEYWORD_IMPL(Name, event, T)

#define DECLARE_KEYWORD(Name, T) \
    DECLARE_KEYWORD_IMPL(Name, global, T)

namespace blackhole {

namespace keyword {

template<typename T, typename NameProvider, log::attribute::scope Scope = log::attribute::DEFAULT_SCOPE>
struct keyword_t {
    typedef T type;

    static const char* name() {
        return NameProvider::name();
    }

    log::attribute_pair_t operator =(T value) const {
        return attribute::make(name(), attribute::traits<T>::pack(value), Scope);
    }

    filter_t operator >=(T value) const {
        return action_t<helper::LessEqThan>({ value });
    }

    filter_t operator ==(T value) const {
        return action_t<helper::Eq>({ value });
    }

    template<typename Action>
    struct action_t {
        T value;

        bool operator()(const log::attributes_t& attributes) const {
            return Action::execute(value, attribute::traits<T>::extract(attributes, name()));
        }
    };
};

} // namespace keyword

} // namespace blackhole
