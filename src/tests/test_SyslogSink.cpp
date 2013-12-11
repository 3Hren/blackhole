#include "Mocks.hpp"

namespace testing {

enum class level : std::uint8_t { debug, info, warn, error };

} // namespace testing

namespace blackhole { namespace sink {

template<>
struct priority_traits<testing::level> {
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
    }
};

} } // namespace blackhole::sink

TEST(syslog_t, Class) {
    sink::syslog_t<testing::level> sink("identity");
    UNUSED(sink);
}

TEST(syslog_t, ConsumeAcceptsLevel) {
    sink::syslog_t<testing::level> sink("identity");
    sink.consume(testing::level::debug, "le message");
}

TEST(syslog_t, MapsLevelViaCustomMapper) {
    sink::syslog_t<testing::level, mock::syslog::backend_t> sink("identity");
    InSequence s;
    EXPECT_CALL(sink.backend(), write(sink::priority_t::debug, std::string("debug message")));
    EXPECT_CALL(sink.backend(), write(sink::priority_t::info, std::string("info message")));
    EXPECT_CALL(sink.backend(), write(sink::priority_t::warning, std::string("warn message")));
    EXPECT_CALL(sink.backend(), write(sink::priority_t::err, std::string("error message")));

    sink.consume(testing::level::debug, "debug message");
    sink.consume(testing::level::info, "info message");
    sink.consume(testing::level::warn, "warn message");
    sink.consume(testing::level::error, "error message");
}
