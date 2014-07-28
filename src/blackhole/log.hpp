#pragma once

#include <type_traits>
#include <tuple>
#include <ostream>

#include "blackhole/detail/traits/attributes/pack/feeder.hpp"

#include "format/message/extraction.hpp"
#include "format/message/insitu.hpp"
#include "logger.hpp"
#include "utils/format.hpp"

namespace blackhole {

namespace aux {

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

    /*
       Using initializer lists is problematic, since GCC 4.6 has an extension which can
       deduce { { "value", "42" } } expressions in function call to the initializer list
       without looking at the overload resolution. This violates C++11 standard, which
       allows such deducing only for `auto` keyword (including range-based `for` loop).
       Please, specify `attribute::list` type explicitly when using initializer lists to
       provide attributes.
    */
    scoped_pump& operator()(std::initializer_list<std::pair<std::string, log::attribute_value_t>>&& args) {
        for (auto it = args.begin(); it != args.end(); ++it) {
            record.attributes.insert(std::make_pair(it->first, log::attribute_t(it->second)));
        }

        return *this;
    }

    template<typename... Args>
    scoped_pump& operator()(Args&&... args) {
        static_assert((is_keyword_pack<Args...>::value || is_emplace_pack<Args...>::value),
                      "parameter pack must be either attribute pack or emplace pack");
        typedef typename pack_determiner<Args...>::type pack_type;
        pack_feeder<pack_type>::feed(record, std::forward<Args>(args)...);

        return *this;
    }
};

template<typename Log, typename... Args>
scoped_pump<Log> make_scoped_pump(Log& log, log::record_t& record, Args&&... args) {
    return scoped_pump<Log>(log, record, std::forward<Args>(args)...);
}

} // namespace aux

} // namespace blackhole

#define BH_LOG(__log__, __level__, ...) \
    if (blackhole::log::record_t record = (__log__).open_record((__level__))) \
        blackhole::aux::make_scoped_pump((__log__), record, __VA_ARGS__)
