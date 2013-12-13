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

namespace aux {

template<typename T, class = void>
struct underlying_type {
    typedef T type;
};

template<typename T>
struct underlying_type<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename std::underlying_type<T>::type type;
};

} // namespace aux

template<typename T, typename NameProvider, log::attribute::scope Scope = log::attribute::DEFAULT_SCOPE>
struct keyword_t {
    typedef typename aux::underlying_type<T>::type type;

    static const char* name() {
        return NameProvider::name();
    }

    log::attribute_pair_t operator =(T value) const {
        return std::make_pair(name(), log::attribute_t(attribute::traits<T>::pack(value), Scope));
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
