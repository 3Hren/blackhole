#include "Mocks.hpp"

enum class level : std::uint8_t { debug, info, warn, error };

namespace blackhole { namespace sink {

template<>
struct priority_traits<level> {
    static inline
    priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::info:
            return priority_t::info;
        case level::warn:
            return priority_t::warning;
        case level::error:
            return priority_t::err;
        }
    }
};

} } // namespace blackhole::sink

TEST(syslog_t, Class) {
    sink::syslog_t<level> sink("identity");
    UNUSED(sink);
}

TEST(syslog_t, ConsumeAcceptsLevel) {
    sink::syslog_t<level> sink("identity");
    sink.consume(level::debug, "le message");
}

TEST(syslog_t, MapsLevelViaCustomMapper) {
    sink::syslog_t<level, mock::syslog::backend_t> sink("identity");
    InSequence s;
    EXPECT_CALL(sink.backend(), write(sink::priority_t::debug, std::string("debug message")));
    EXPECT_CALL(sink.backend(), write(sink::priority_t::info, std::string("info message")));
    EXPECT_CALL(sink.backend(), write(sink::priority_t::warning, std::string("warn message")));
    EXPECT_CALL(sink.backend(), write(sink::priority_t::err, std::string("error message")));

    sink.consume(level::debug, "debug message");
    sink.consume(level::info, "info message");
    sink.consume(level::warn, "warn message");
    sink.consume(level::error, "error message");
}
