#include <blackhole/logger.hpp>
#include <blackhole/detail/logger/pusher.hpp>

#include "../global.hpp"
#include "../mocks/logger.hpp"

using namespace blackhole;

TEST(pusher_t, Constructor) {
    mock::verbose_log_t<level> log;
    record_t record;
    const char* msg = "test msg";
    aux::logger::pusher_t<decltype(log)> pusher(log, record, msg);
    EXPECT_CALL(log, push(_))
        .WillOnce(Throw(std::logic_error("mock exception")));
}

TEST(pusher_t, VarArgsConstructor) {
    mock::verbose_log_t<level> log;
    record_t record;
    const char* msg = "test msg. Data %s";
    const char* data = "DATA";
    aux::logger::pusher_t<decltype(log)> pusher(log, record, msg, data);
    EXPECT_CALL(log, push(_))
        .WillOnce(Throw(std::logic_error("mock exception")));
}
