#include <blackhole/formatter/string.hpp>
#include <blackhole/frontend/syslog.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/sink/syslog.hpp>

#include "global.hpp"

using namespace blackhole;

namespace blackhole { namespace sink {

template<>
struct priority_traits<level> {
    static inline
    priority_t map(testing::level lvl) {
        switch (lvl) {
        case testing::level::debug:
            return priority_t::debug;
        case testing::level::info:
            return priority_t::info;
        case testing::level::warn:
            return priority_t::warning;
        case testing::level::error:
            return priority_t::err;
        }

        return priority_t::debug;
    }
};

} } // namespace blackhole::sink

TEST(Functional, SyslogConfiguredVerboseLogger) {
    verbose_logger_t<level> log;

    typedef formatter::string_t formatter_type;
    typedef sink::syslog_t<level> sink_type;

    auto formatter = utils::make_unique<formatter_type>("%(message)s [%(...L)s]");
    auto sink = utils::make_unique<sink_type>("testing");
    auto frontend = utils::make_unique<frontend_t<formatter_type, sink_type>>(std::move(formatter), std::move(sink));
    log.add_frontend(std::move(frontend));

    log::record_t record = log.open_record(level::error);
    if (record.valid()) {
        record.attributes.insert(keyword::message() = utils::format("Some message from: '%s'!", "Hell"));
        log.push(std::move(record));
    }
}
