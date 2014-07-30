#pragma once

#include "blackhole/detail/logger/pusher.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/sink/stream.hpp"
#include "blackhole/synchronized.hpp"
#include "blackhole/utils/unused.hpp"

#ifdef ENABLE_ELASTICSEARCH_DEBUG
#define ES_LOG(__log__, ...) \
    if (auto record = (__log__).open_record()) \
        ::blackhole::aux::make_pusher((__log__), record, __VA_ARGS__)
#else
#define ES_LOG(__log__, ...) \
    ::blackhole::utils::ignore_unused_variable_warning((__log__), __VA_ARGS__)
#endif

namespace elasticsearch {

class logger_factory_t {
public:
    static blackhole::logger_base_t create() {
        blackhole::logger_base_t logger;
        auto formatter = blackhole::utils::make_unique<
            blackhole::formatter::string_t
        >("[%(timestamp)s] [%(tid)s]: %(message)s");

        auto sink = blackhole::utils::make_unique<
            blackhole::sink::stream_t
        >(blackhole::sink::stream_t::output_t::stdout);

        auto frontend = blackhole::utils::make_unique<
            blackhole::frontend_t<
                blackhole::formatter::string_t,
                blackhole::sink::stream_t
            >
        >(std::move(formatter), std::move(sink));

        logger.add_frontend(std::move(frontend));
        return logger;
    }
};

} // namespace elasticsearch
