#include <gtest/gtest.h>

#include <blackhole/record.hpp>

namespace blackhole {
namespace testing {

TEST(Record, Severity) {
    const view_of<attributes_t>::type attributes{{"key#1", {42}}};
    attribute_pack pack{};

    record_t record(42, "GET /porn.png HTTP/1.1", pack);

    EXPECT_EQ(42, record.severity());
}

TEST(Record, Message) {
    const view_of<attributes_t>::type attributes{{"key#1", {42}}};
    attribute_pack pack{};

    record_t record(42, "GET /porn.png HTTP/1.1", pack);

    EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
}

}  // namespace testing
}  // namespace blackhole
