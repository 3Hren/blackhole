#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
namespace testing {
namespace formatter {

using ::blackhole::formatter::json_t;
using ::blackhole::formatter::routing_t;

TEST(json_t, FormatMessage) {
    json_t formatter;

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ(R"({"message":"value"})", writer.result().to_string());
}

TEST(json_t, FormatAttribute) {
    json_t formatter;

    const string_view message("value");
    const attribute_list attributes{{"counter", 42}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("counter"));
    ASSERT_TRUE(doc["counter"].IsInt());
    EXPECT_EQ(42, doc["counter"].GetInt());
}

TEST(json_t, FormatAttributeString) {
    json_t formatter;

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("endpoint"));
    ASSERT_TRUE(doc["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["endpoint"].GetString());
}

TEST(json_t, MessageRouting) {
    json_t formatter(routing_t().spec("/fields", {"message"}));

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ(R"({"fields":{"message":"value"}})", writer.result().to_string());
}

}  // namespace formatter
}  // namespace testing
}  // namespace blackhole
