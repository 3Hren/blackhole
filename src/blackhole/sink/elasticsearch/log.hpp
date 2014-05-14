#pragma once

#include "blackhole/formatter/string.hpp"
#include "blackhole/log.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/sink/stream.hpp"
#include "blackhole/synchronized.hpp"

#define LOG(__log__, ...) \
    if (blackhole::log::record_t record = __log__.open_record()) \
        blackhole::aux::make_scoped_pump(__log__, record, __VA_ARGS__)

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
