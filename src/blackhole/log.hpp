#pragma once

#include <type_traits>
#include <tuple>
#include <ostream>

#include "blackhole/detail/traits/literal.hpp"
#include "blackhole/detail/traits/or.hpp"
#include "blackhole/detail/traits/same.hpp"
#include "blackhole/detail/traits/tuple.hpp"
#include "blackhole/detail/traits/supports/stream_push.hpp"

#include "format/message/extraction.hpp"
#include "format/message/insitu.hpp"
#include "logger.hpp"
#include "utils/format.hpp"

namespace blackhole {

template<typename T>
struct is_convertible : public std::conditional<
        log::attribute::is_constructible<T>::value || traits::supports::stream_push<T>::value,
        std::true_type,
        std::false_type
    >
{};

namespace aux {

template<class... Args>
struct is_keyword_pack {
    static const bool value = are_same<log::attribute_pair_t, Args...>::value;
};

template<class... Args>
struct all_first_string_literal {
    static const bool value = tuple::all<
        typename tuple::map<
            is_string_literal_type,
            typename tuple::remove_index<
                typename tuple::filter<
                    tuple::slice<0, -1, 2>::type,
                    typename tuple::add_index<
                        std::tuple<Args...>
                    >::type
                >::type
            >::type
        >::type
    >::value;
};

template<class... Args>
struct all_second_constructible {
    static const bool value = tuple::all<
        typename tuple::map<
            is_convertible,
            typename tuple::remove_index<
                typename tuple::filter<
                    tuple::slice<1, -1, 2>::type,
                    typename tuple::add_index<
                        std::tuple<Args...>
                    >::type
                >::type
            >::type
        >::type
    >::value;
};

template<class... Args>
struct is_emplace_pack {
    static const bool value =
            sizeof...(Args) % 2 == 0 &&
            all_first_string_literal<Args...>::value &&
            all_second_constructible<Args...>::value;
};

template<typename T, class = void>
struct conv;

template<typename T>
struct conv<T, typename std::enable_if<
        log::attribute::is_constructible<T>::value>::type
    >
{
    static log::attribute_value_t from(T&& value) {
        return log::attribute_value_t(std::forward<T>(value));
    }
};

template<typename T>
struct conv<T, typename std::enable_if<
        !log::attribute::is_constructible<T>::value &&
        traits::supports::stream_push<T>::value>::type
    >
{
    static log::attribute_value_t from(T&& value) {
        std::ostringstream stream;
        stream << value;
        return log::attribute_value_t(stream.str());
    }
};

template<typename T>
struct conv<T, typename std::enable_if<
        !log::attribute::is_constructible<T>::value &&
        !traits::supports::stream_push<T>::value>::type
    >
{
    static log::attribute_value_t from(T&&) {
        static_assert(lazy_false<T>::value, "stream operator<< is not defined for type `T`");
        return log::attribute_value_t();
    }
};

template<bool, class... Args>
struct selector;

template<class... Args>
struct selector<true, Args...> { //!@todo: true/false is not so readable.
    static void action(log::record_t& record, Args&&... args) {
        record.fill(std::forward<Args>(args)...);
    }
};

template<class... Args>
struct selector<false, Args...> {
    template<class T, class... Tail>
    static void action(log::record_t& record, const char* name, T&& value, Tail&&... args) {
        record.attributes.insert(std::make_pair(name, log::attribute_t(conv<T>::from(std::forward<T>(value)))));
        action(record, std::forward<Tail>(args)...);
    }

    static void action(log::record_t&) {}
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
                      "parameter pack must be either attribute pack or emplace pack");
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
