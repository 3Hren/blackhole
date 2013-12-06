#pragma once

#include "attribute.hpp"
#include "filter.hpp"

#define DECLARE_KEYWORD(Name, T) \
    namespace tag { \
        struct Name##_t { \
            static const char* name() { return #Name; } \
        }; \
    } \
    static blackhole::keyword::keyword_t<T, tag::Name##_t>& Name() { \
        static blackhole::keyword::keyword_t<T, tag::Name##_t> self; \
        return self; \
    }

namespace blackhole {

namespace keyword {

template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename std::underlying_type<T>::type underlying_type;

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name).value));
    }
};

template<typename T, class = void>
struct type_extracter {
    typedef T type;
};

template<typename T>
struct type_extracter<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename std::underlying_type<T>::type type;
};

//!@todo: Need testing.
template<typename T, typename NameProvider>
struct keyword_t {
    typedef typename type_extracter<T>::type type;

    static const char* name() {
        return NameProvider::name();
    }

    log::attribute_pair_t operator =(T value) const {
        return std::make_pair(name(), log::attribute_t(traits<T>::pack(value), log::attribute_t::type_t::local));
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
            return Action::execute(value, traits<T>::extract(attributes, name()));
        }
    };
};

//!@todo: Maybe replace to the separate file 'keywords/severity.hpp'?
namespace tag {

struct severity_t {
    static const char* name() { return "severity"; }
};

} // namespace tag

template<typename T>
static keyword_t<T, tag::severity_t>& severity() {
    static keyword_t<T, tag::severity_t> self;
    return self;
}

} // namespace keyword

} // namespace blackhole

namespace blackhole { namespace keyword {
DECLARE_KEYWORD(timestamp_id, std::time_t)
} }
