#include "Mocks.hpp"

using namespace blackhole;

TEST(msgpack_t, Class) {
    formatter::msgpack_t fmt;
    UNUSED(fmt);
}

TEST(msgpack_t, _) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };

    formatter::msgpack_t fmt;
    fmt.format(record);

    EXPECT_EQ("\x81\xa7message\xaale message", fmt.format(record));
}
