#include <blackhole/blackhole.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/frontend/syslog.hpp>
#include <blackhole/scoped_attributes.hpp>
#include <blackhole/sink/stream.hpp>
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
    verbose_logger_t<level> log(level::debug);

    typedef formatter::string_t formatter_type;
    typedef sink::syslog_t<level> sink_type;

    auto formatter = aux::util::make_unique<formatter_type>("%(message)s [%(...L)s]");
    auto sink = aux::util::make_unique<sink_type>("testing");
    auto frontend = aux::util::make_unique<frontend_t<formatter_type, sink_type>>(std::move(formatter), std::move(sink));
    log.add_frontend(std::move(frontend));

    record_t record = log.open_record(level::error);
    if (record.valid()) {
        record.insert(keyword::message() = utils::format("Some message from: '%s'!", "Hell"));
        log.push(std::move(record));
    }
}

TEST(Functional, LoggerShouldProperlyRouteAttributesByScope) {
    /*
     * We define full Blackhole's util stack: logger, wrapper, scoped attributes
     * and user specific attributes.
     * We expect, that all external attributes appear in variadic placeholder.
     * External attributes are:
     *  - Logger attached.
     *  - Wrapper attached.
     *  - Scoped.
     *  - User defined (via macro)
     */
    using aux::util::make_unique;

    typedef formatter::string_t                   formatter_type;
    typedef sink::stream_t                        sink_type;
    typedef frontend_t<formatter_type, sink_type> frontend_type;
    typedef verbose_logger_t<level>               logger_type;

    std::ostringstream stream;
    logger_type log(level::debug);

    auto formatter = make_unique<formatter_type>("[%(severity)s]: %(message)s %(...:[:])s");
    auto sink      = make_unique<sink_type>(stream);
    auto frontend  = make_unique<frontend_type>(std::move(formatter), std::move(sink));
    log.add_frontend(std::move(frontend));

    wrapper_t<logger_type> wrapper(log, attribute::set_t({{ "w1", attribute_t(42) }}));

    scoped_attributes_t scope(wrapper, attribute::set_t({{ "s1", attribute_t(10) }}));

    BH_LOG(wrapper, level::debug, "message is so %s", "message")("u1", "value");

    auto actual = stream.str();
    EXPECT_THAT(actual, AnyOf(
        Eq("[0]: message is so message [w1: 42, s1: 10, u1: value]\n"),
        Eq("[0]: message is so message [w1: 42, u1: value, s1: 10]\n"),
        Eq("[0]: message is so message [u1: value, s1: 10, w1: 42]\n"),
        Eq("[0]: message is so message [u1: value, w1: 42, s1: 10]\n"),
        Eq("[0]: message is so message [s1: 10, u1: value, w1: 42]\n"),
        Eq("[0]: message is so message [s1: 10, w1: 42, u1: value]\n")
    ));
}
