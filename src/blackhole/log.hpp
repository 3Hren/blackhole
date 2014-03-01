#pragma once

#include <type_traits>
#include <tuple>

#include <boost/integer_traits.hpp>

#include "blackhole/detail/traits/same.hpp"

#include "format/message/extraction.hpp"
#include "format/message/insitu.hpp"
#include "logger.hpp"
#include "utils/format.hpp"

namespace blackhole {

namespace aux {

template<class... Args>
struct is_keyword_pack {
    static const bool value = are_same<log::attribute_pair_t, Args...>::value;
};

template<class IndexedSequence>
struct odd;

template<int N, typename T>
struct odd<std::tuple<std::integral_constant<int, N>, T>> {
    static const bool value = N % 2 == 1;
};

template<class IndexedSequence>
struct even;

template<int N, typename T>
struct even<std::tuple<std::integral_constant<int, N>, T>> {
    static const bool value = N % 2 == 0;
};

template<int START, int STOP = -1, int STEP = 1>
struct slice {
    template<class IndexedSequence>
    struct type;

    template<int N, typename T>
    struct type<std::tuple<std::integral_constant<int, N>, T>> {
        static const int delta = N - START;
        static const int stop = STOP == -1 ? boost::integer_traits<int>::const_max : STOP;
        static const bool value = delta >= 0 && N < stop && delta % STEP == 0;
    };
};

template<class Sequence, class OtherSequence>
struct concat;

template<typename... Sequence, typename... OtherSequence>
struct concat<std::tuple<Sequence...>, std::tuple<OtherSequence...>> {
    typedef std::tuple<Sequence..., OtherSequence...> type;
};

template<int N, int MAX, class Sequence>
struct add_index_impl;

template<int MAX>
struct add_index_impl<MAX, MAX, std::tuple<>> {
    typedef std::tuple<> type;
};

template<int N, int MAX, typename T, typename... Args>
struct add_index_impl<N, MAX, std::tuple<T, Args...>> {
    typedef typename concat<
        std::tuple<std::tuple<std::integral_constant<int, N>, T>>,
        typename add_index_impl<N + 1, MAX, std::tuple<Args...>>::type
    >::type type;
};

template<class Sequence>
struct add_index;

template<typename... Args>
struct add_index<std::tuple<Args...>> {
    typedef typename add_index_impl<0, sizeof...(Args), std::tuple<Args...>>::type type;
};

template<class IndexedSequence>
struct remove_index;

template<>
struct remove_index<std::tuple<>> {
    typedef std::tuple<> type;
};

template<int N, typename T, typename... Args>
struct remove_index<std::tuple<std::tuple<std::integral_constant<int, N>, T>, Args...>> {
    typedef typename concat<
        std::tuple<T>,
        typename remove_index<std::tuple<Args...>>::type
    >::type type;
};

template<template<typename> class F, class Sequence>
struct filter_impl;

template<template<typename> class F>
struct filter_impl<F, std::tuple<>> {
    typedef std::tuple<> type;
};

template<template<typename> class F, typename T, typename... Args>
struct filter_impl<F, std::tuple<T, Args...>> {
    typedef typename std::conditional<
        F<T>::value,
        typename concat<std::tuple<T>, typename filter_impl<F, std::tuple<Args...>>::type>::type,
        typename filter_impl<F, std::tuple<Args...>>::type
    >::type type;
};

template<template<typename> class F, class Sequence>
struct filter;

template<template<typename> class F, typename... Args>
struct filter<F, std::tuple<Args...>> {
    typedef typename filter_impl<F, std::tuple<Args...>>::type type;
};

template<template<typename> class F, class Sequence>
struct map;

template<template<typename> class F, typename... Args>
struct map<F, std::tuple<Args...>>{
    typedef std::tuple<typename F<Args>::type...> type;
};

template<class T>
struct is_true_type {
    static const bool value = std::is_same<T, std::true_type>::value;
};

template<class Sequence, template<class> class F = is_true_type>
struct all;

template<template<class> class F>
struct all<std::tuple<>, F> {
    static const bool value = true;
};

template<template<class> class F, typename T, typename... Args>
struct all<std::tuple<T, Args...>, F> {
    static const bool value = !F<T>::value ? false : all<std::tuple<Args...>, F>::value;
};

template<class, class>
struct or_ : public std::true_type {};

template<>
struct or_<std::false_type, std::false_type> : public std::false_type {};

template<typename T>
struct is_string_literal_type {
    typedef typename or_<
        typename std::is_same<
            const char*,
            typename std::decay<T>::type
        >::type,
        typename std::is_same<
            char*,
            typename std::decay<T>::type
        >::type
    >::type type;
};

template<class... Args>
struct all_odd_string_literal {
    static const bool value = all<
        typename map<
            is_string_literal_type,
            typename remove_index<
                typename filter<
                    even,
                    typename add_index<
                        std::tuple<Args...>
                    >::type
                >::type
            >::type
        >::type
    >::value;
};

//template<class T>
//struct all_odd_string_literal<T> : public std::true_type {};

//template<class T, class Odd>
//struct all_odd_string_literal<T, Odd> : public std::false_type {};

//template<class T, class Odd, class Even, class... Args>
//struct all_odd_string_literal<T, Odd, Even, Args...> {
//    static const bool value =
//            std::is_same<T, typename std::decay<Odd>::type>::value &&
//            all_odd_string_literal<T, Args...>::value;
//};

template<class... Args>
struct all_even_constructible;

template<>
struct all_even_constructible<> : public std::true_type {};

template<class Odd>
struct all_even_constructible<Odd> : public std::false_type {};

template<class Odd, class Even, class... Args>
struct all_even_constructible<Odd, Even, Args...> {
    static const bool value =
            log::attribute::is_constructible<typename std::decay<Even>::type>::value &&
            all_even_constructible<Args...>::value;
};

template<class... Args>
struct is_emplace_pack {
    static const bool value =
            sizeof...(Args) % 2 == 0 &&
            all_odd_string_literal<Args...>::value &&
            all_even_constructible<Args...>::value;
};

template<bool, class... Args>
struct selector;

template<class... Args>
struct selector<true, Args...> {
    static void action(log::record_t& record, Args&&... args) {
        record.fill(std::forward<Args>(args)...);
    }
};

template<class... Args>
struct selector<false, Args...> {
    static void action(log::record_t& record, Args&&... args) {
        action_impl(record, std::forward<Args>(args)...);
    }

    template<class T, class... Args2>
    static void action_impl(log::record_t& record, const char* name, T&& t, Args2&&... args) {
        record.attributes.insert(std::make_pair(name, log::attribute_t(std::forward<T>(t))));
        action_impl(record, std::forward<Args2>(args)...);
    }

    static void action_impl(log::record_t&) {}
};

template<typename Log>
class scoped_pump {
    const Log& log;
    log::record_t& record;

public:
    template<typename... Args>
    scoped_pump(const Log& log, log::record_t& record, Args&&... args) :
        log(log),
        record(record)
    {
        record.attributes.insert(keyword::message() = aux::format(record.attributes, std::forward<Args>(args)...));
    }

    ~scoped_pump() {
        log.push(std::move(record));
    }

    template<typename... Args>
    void operator()(Args&&... args) {
        static_assert((is_keyword_pack<Args...>::value || is_emplace_pack<Args...>::value),
                      "parameter pack must be either attribute pack or emplace pairs");
        selector<
            is_keyword_pack<Args...>::value,
            Args...
        >::action(record, std::forward<Args>(args)...);
    }
};

template<typename Log, typename... Args>
scoped_pump<Log> make_scoped_pump(Log& log, log::record_t& record, Args&&... args) {
    return scoped_pump<Log>(log, record, std::forward<Args>(args)...);
}

} // namespace aux

} // namespace blackhole

#define BH_LOG(__log__, level, ...) \
    for (blackhole::log::record_t record = __log__.open_record(level); record.valid(); record.attributes.clear()) \
        blackhole::aux::make_scoped_pump(__log__, record, __VA_ARGS__)
