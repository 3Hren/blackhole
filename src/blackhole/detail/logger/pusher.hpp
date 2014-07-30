#pragma once

#include <type_traits>
#include <tuple>
#include <ostream>

#include "blackhole/detail/traits/attributes/pack/feeder.hpp"
#include "blackhole/format/message/extraction.hpp"
#include "blackhole/format/message/insitu.hpp"
#include "blackhole/keyword/message.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace aux {

namespace logger {

template<typename Log>
class pusher_t {
public:
    typedef Log logger_type;

private:
    const logger_type& log;
    log::record_t& record;

public:
    template<typename... Args>
    pusher_t(const logger_type& log, log::record_t& record, Args&&... args) :
        log(log),
        record(record)
    {
        //!@todo: Catch exceptions from message inline formatting.
        record.attributes.insert(
            keyword::message() =
                aux::format(record.attributes, std::forward<Args>(args)...)
        );
    }

    ~pusher_t() {
        log.push(std::move(record));
    }

    /*!
     * Using initializer lists is problematic, since GCC 4.6 has an extension
     * which can deduce {{ "value", "42" }} expressions in function call to
     * the initializer list without looking at the overload resolution.
     * This behaviour violates C++11 standard, which allows such deducing only
     * for `auto` keyword (including range-based `for` loop).
     * Please, specify `attribute::list` type explicitly when using initializer
     * lists to provide attributes.
     */
    pusher_t&
    operator()(std::initializer_list<
                   std::pair<std::string, log::attribute_value_t>
               >&& args)
    {
        for (auto it = args.begin(); it != args.end(); ++it) {
            record.attributes.insert(
                std::make_pair(it->first, log::attribute_t(it->second))
            );
        }

        return *this;
    }

    template<typename... Args>
    pusher_t&
    operator()(Args&&... args) {
        static_assert(
            (is_keyword_pack<Args...>::value || is_emplace_pack<Args...>::value),
            "parameter pack must be either attribute pack or emplace pack"
        );
        typedef typename pack_determiner<Args...>::type pack_type;
        pack_feeder<pack_type>::feed(record, std::forward<Args>(args)...);

        return *this;
    }
};

template<typename Log, typename... Args>
pusher_t<Log>
make_pusher(Log& log, log::record_t& record, Args&&... args) {
    return pusher_t<Log>(log, record, std::forward<Args>(args)...);
}

} // namespace logger

} // namespace aux

} // namespace blackhole
