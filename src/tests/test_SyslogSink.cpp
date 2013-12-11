#include "Mocks.hpp"

TEST(syslog_t, Class) {
    sink::syslog_t<> sink("identity");
    UNUSED(sink);
}

TEST(syslog_t, ConsumeAcceptsPriority) {
    sink::syslog_t<> sink("identity");
    sink.consume(0, "le message");
}
