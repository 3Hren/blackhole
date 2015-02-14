#pragma once

#include <exception>
#include <ostream>
#include <tuple>
#include <type_traits>

#include "blackhole/detail/traits/attributes/pack/feeder.hpp"
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
    record_t& record;

public:
    pusher_t(const logger_type& log, record_t& record, const char* message) :
        log(log),
        record(record)
    {
        record.message(message);
    }

    template<typename... Args>
    pusher_t(const logger_type& log, record_t& record, const char* message, Args&&... args) :
        log(log),
        record(record)
    {
        record.message(this->message(message, std::forward<Args>(args)...));
    }

    ~pusher_t() {
        //In Debug builds push can throw.
        try {
            log.push(std::move(record));
        }
        catch (const std::exception& e) {
            std::cout << "Exception caught in pusher destructor: " << e.what() << std::endl;
        }
    }

    /*!
     * \compat GCC 4.6
     * Using initializer lists is problematic, since GCC 4.6 has an extension (enabled by default)
     * which can deduce {{ "value", "42" }} expressions in function call to the initializer list
     * without looking at the overload resolution. This behaviour violates C++11 standard, which
     * allows such deducing only for `auto` keyword (including range-based `for` loop).
     * Please, specify `attribute::list` type explicitly when using initializer lists to provide
     * attributes.
     */
    pusher_t&
    operator()(std::initializer_list<std::pair<std::string, attribute::value_t>>&& args) {
        for (auto it = args.begin(); it != args.end(); ++it) {
            record.insert(std::make_pair(it->first, attribute_t(it->second)));
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

private:
    template<typename... Args>
    static
    inline
    std::string
    message(const char* message, Args&&... args) {
        return utils::format(message, std::forward<Args>(args)...);
    }
};

template<typename Log, typename... Args>
pusher_t<Log>
make_pusher(Log& log, record_t& record, Args&&... args) {
    return pusher_t<Log>(log, record, std::forward<Args>(args)...);
}

} // namespace logger

} // namespace aux

} // namespace blackhole
