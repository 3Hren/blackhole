#include "Mocks.hpp"

using namespace blackhole;

TEST(msgpack_t, Class) {
    formatter::msgpack_t fmt;
    UNUSED(fmt);
}

TEST(msgpack_t, FormatSingleAttribute) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };

    formatter::msgpack_t fmt;
    fmt.format(record);

    EXPECT_EQ("\x81\xa7message\xaale message", fmt.format(record));
}

TEST(msgpack_t, FormatMultipleAttributes) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = 100500
    };

    formatter::msgpack_t fmt;
    fmt.format(record);

    EXPECT_EQ("\x82\xa9timestamp\xce\x00\x01\x88\x94\xa7message\xaale message", fmt.format(record));
}

//!@todo: Nested attribute objects. It is needed for logstash.
