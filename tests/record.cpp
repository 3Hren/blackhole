#include <gtest/gtest.h>

#include <blackhole/record.hpp>

namespace blackhole {
namespace testing {

TEST(Record, Severity) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    record_t record(42, message, pack);

    EXPECT_EQ(42, record.severity());
}

TEST(Record, Message) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    record_t record(42, message, pack);

    EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
}

TEST(Record, Attributes) {
    const string_view message("GET /porn.png HTTP/1.1");
    const view_of<attributes_t>::type attributes{{"key#1", {42}}};
    const attribute_pack pack{attributes};

    record_t record(42, message, pack);

    ASSERT_EQ(1, record.attributes().size());
    EXPECT_EQ(attributes, record.attributes().at(0).get());
}

TEST(Record, Pid) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    record_t record(42, message, pack);

    EXPECT_EQ(::getpid(), record.pid());
}

TEST(Record, Tid) {
    std::thread::native_handle_type tid;
    std::thread thread([&] {
        const string_view message("GET /porn.png HTTP/1.1");
        const attribute_pack pack;

        record_t record(42, message, pack);
        tid = record.tid();
    });

    auto handle = thread.native_handle();
    thread.join();

    EXPECT_EQ(handle, tid);
}

TEST(Record, NullTimestampByDefault) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    record_t record(42, message, pack);

    EXPECT_EQ(record_t::time_point(), record.timestamp());
}

TEST(Record, Timestamp) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    const auto min = record_t::clock_type::now();
    record_t record(42, message, pack);
    record.activate();
    const auto max = record_t::clock_type::now();

    EXPECT_TRUE(min <= record.timestamp());
    EXPECT_TRUE(max >= record.timestamp());
}

TEST(Record, FormattedEqualsMessageByDefault) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    record_t record(42, message, pack);
    record.activate();

    EXPECT_EQ("GET /porn.png HTTP/1.1", record.formatted().to_string());
}

TEST(Record, Formatted) {
    const string_view message("GET /porn.png HTTP/1.1");
    const string_view formatted("GET /porn.png HTTP/1.1 - SUCCESS");
    const attribute_pack pack;

    record_t record(42, message, pack);
    record.activate(formatted);

    EXPECT_EQ("GET /porn.png HTTP/1.1 - SUCCESS", record.formatted().to_string());
}

}  // namespace testing
}  // namespace blackhole
