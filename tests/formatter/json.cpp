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

TEST(json_t, FormatMessage) {
    json_t formatter;

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());
}

TEST(json_t, FormatSeverity) {
    json_t formatter;

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("severity"));
    ASSERT_TRUE(doc["severity"].IsInt());
    EXPECT_EQ(4, doc["severity"].GetInt());
}

TEST(json_t, FormatTimestamp) {
    json_t formatter;

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    record.activate();
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    ASSERT_TRUE(doc["timestamp"].IsUint64());
    EXPECT_TRUE(doc["timestamp"].GetUint64() > 0);
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

TEST(json_t, FormatAttributeNull) {
    json_t formatter;

    const string_view message("value");
    const attribute_list attributes{{"endpoint", nullptr}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("endpoint"));
    ASSERT_TRUE(doc["endpoint"].IsNull());
}

TEST(json_t, FormatAttributeBool) {
    json_t formatter;

    const string_view message("value");
    const attribute_list attributes{{"available", true}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("available"));
    ASSERT_TRUE(doc["available"].IsBool());
    EXPECT_TRUE(doc["available"].GetBool());
}

TEST(json_t, FormatAttributeDouble) {
    json_t formatter;

    const string_view message("value");
    const attribute_list attributes{{"pi", 3.1415}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("pi"));
    ASSERT_TRUE(doc["pi"].IsDouble());
    EXPECT_DOUBLE_EQ(3.1415, doc["pi"].GetDouble());
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

TEST(json_t, FormatMessageWithRouting) {
    auto formatter = json_t::builder_t()
        .route("/fields", {"message"})
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("message"));
    ASSERT_TRUE(doc["fields"]["message"].IsString());
    EXPECT_STREQ("value", doc["fields"]["message"].GetString());
}

TEST(json_t, FormatAttributeStringWithRouting) {
    auto formatter = json_t::builder_t()
        .route("/fields", {"endpoint"})
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("endpoint"));
    ASSERT_TRUE(doc["fields"]["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["fields"]["endpoint"].GetString());
}

TEST(json_t, FormatAttributeStringWithNestedRouting) {
    auto formatter = json_t::builder_t()
        .route("/fields/external", {"endpoint"})
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("external"));
    ASSERT_TRUE(doc["fields"]["external"].IsObject());
    ASSERT_TRUE(doc["fields"]["external"].HasMember("endpoint"));
    ASSERT_TRUE(doc["fields"]["external"]["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["fields"]["external"]["endpoint"].GetString());
}

TEST(json_t, FormatMessageWithRenaming) {
    auto formatter = json_t::builder_t()
        .rename("message", "#message")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("#message"));
    ASSERT_TRUE(doc["#message"].IsString());
    EXPECT_STREQ("value", doc["#message"].GetString());
}

TEST(json_t, FormatAttributeWithRenaming) {
    auto formatter = json_t::builder_t()
        .rename("source", "#source")
        .build();

    const string_view message("value");
    const attribute_list attributes{{"source", "storage"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("#source"));
    ASSERT_TRUE(doc["#source"].IsString());
    EXPECT_STREQ("storage", doc["#source"].GetString());
}

}  // namespace formatter
}  // namespace testing
}  // namespace blackhole
