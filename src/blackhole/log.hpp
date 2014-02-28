#pragma once

#include "format/message/extraction.hpp"
#include "format/message/insitu.hpp"
#include "logger.hpp"
#include "utils/format.hpp"

namespace blackhole {

namespace aux {

template<class...>
struct all_same;

template<class T>
struct all_same<T> {
    static const bool value = true;
};

template<class T, class Arg, class... Args>
struct all_same<T, Arg, Args...> {
    static const bool value = std::is_same<T, Arg>::value && all_same<T, Args...>::value;
};

template<class... Args>
struct is_keyword_pack {
    static const bool value = all_same<log::attribute_pair_t, Args...>::value;
};

template<class... Args>
struct all_odd_same;

template<class T>
struct all_odd_same<T> {
    static const bool value = true;
};

template<class T, class Odd>
struct all_odd_same<T, Odd> {
    static const bool value = false;
};

template<class T, class Odd, class Even, class... Args>
struct all_odd_same<T, Odd, Even, Args...> {
    static const bool value =
            std::is_same<T, typename std::decay<Odd>::type>::value &&
            all_odd_same<T, Args...>::value;
};

template<class... Args>
struct all_even_constructible;

template<class T>
struct all_even_constructible<T> {
    static const bool value = true;
};

template<class T, class Odd>
struct all_even_constructible<T, Odd> {
    static const bool value = false;
};

template<class T, class Odd, class Even, class... Args>
struct all_even_constructible<T, Odd, Even, Args...> {
    static const bool value =
            std::is_constructible<T, typename std::decay<Even>::type>::value &&
            all_even_constructible<T, Args...>::value;
};

template<class... Args>
struct is_emplace_pack {
    static const bool value =
            sizeof...(Args) % 2 == 0 &&
            all_odd_same<const char*, Args...>::value &&
            all_even_constructible<log::attribute_value_t, Args...>::value;
};

template<class... Args> struct Pack {};

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
